#include <iostream>
#include <fstream>
#include <zconf.h>
#include <stdint.h>
#include <ctime>
#include <chrono>
#include "lz77.h"

// КДЗ по дисциплине Алгоритмы и структуры данных 2017-2018 уч.год
// Коршиков Федор Андреевич, дата (08.04.2018)
// Среда разработки: CLion, состав проекта: lz77.cpp lz77.h
// реализован алгоритм сжатия lz77

using namespace std;

CycledCharArray::CycledCharArray(int s)  //s - size (max size)
    {
        size = s;
        arr = new char[size];
        start = 0;
        end = 0;
        curr_length = 0;
        is_full = false;
    }
CycledCharArray::~CycledCharArray()
    {
        delete[] arr;
    }
char CycledCharArray::getValue(int i)
    {
        if (i >= curr_length )
            throw string("CCA::getValue(): out of range");

        return arr[(start + i) % size];
    }

void CycledCharArray::addValueToTail(char val)   // if arr is not full, writes after last el. if full, tail sets to the first el. returns value to be deleted
    {
        if (is_full)
        {
            this->Push(val);
            return;
        }
        if ( curr_length == 0)
        {
            arr[end] = val;
        }
        else
        {
            end = (end + 1) % size;
            arr[end] = val;
        }
        curr_length++;
            if (curr_length == size)
                is_full = true;

        //printArr(); //test
    }

void CycledCharArray::Push(char val)
    {
        start = (start + 1) % size;
        end = (end + 1) % size;
        arr[end] = val;
    }
void CycledCharArray::removeFirst()
    {
        if (curr_length == 0)
            return;

        if (curr_length == 1)
        {
            curr_length--;

        }
        else
        {
            start = (start + 1) % size;
            curr_length--;
            if (is_full)
                is_full = false;
        }

        return;
    }

int CycledCharArray::getLength ()
    {
        return curr_length;
    }

bool CycledCharArray::IsFull()
    {
        return is_full;
    }

void CycledCharArray::printArr ()    // for testing
    {
        int i;
        cout << "CurrentLen=" << curr_length << "Arr:" << endl;
        for (i=0; i<curr_length; i++)
            cout << this->getValue(i) << ' ';
        cout << endl;
    }
Node CycledCharArray::findMatch(CycledCharArray* source)     // returns node with filled length and offset. next is not defined. if match wasnt found, offset = length = 0
    {
        Node n;
        n.length = 0;
        n.offset = 0;

        unsigned short i, j;
        //cout << "!";
        for (i=0; i<curr_length; i++)
        {
            //this->printArr();
            //source->printArr();
            if ( curr_length - i < n.length )   // optimization
                break;

            if ( this->getValue(i) == source->getValue(0) )
            {
                //cout << "i=" << i << " value=" << this->getValue(i) << endl;
                for (j=1; i+j < this->getLength() && j < source->getLength() - 1 ; j++) //  source->getLength() - 1 added, because last symb should be written to next
                    if (this->getValue(i+j) != source->getValue(j))
                        break;
                if (j > n.length)
                {
                    n.length = j;
                    n.offset = curr_length - i ;
                }
            }

        }

        return n;
    }



BitsFileOfNodes::BitsFileOfNodes (string path, char t, unsigned char offb, unsigned char lenb)     //needs to know how much bytes take every field (next is 8bit)
    {
        offbitsn = offb;
        lenbitsn = lenb;
        type = t;

        curr = 0;
        if (type == 'r')    // for reading from file
        {
            inp = new ifstream(path, ifstream::in | ifstream::binary);

            inp->read( (char *) &n_of_nodes, sizeof (unsigned long long) );
            bs = nullptr;
            this->Fill() ;  // fill bitset first time
            nodes_red = 0;
            eof = false;
        }
        else if (type == 'w')   // for writing to file
        {
            outp = new ofstream (path, ofstream::out | ofstream::binary | ofstream::trunc);
            n_of_nodes = 0;
            outp->write( (char *) &n_of_nodes, sizeof(unsigned long long));  // first of all, file contains unsigned long - number of bytes
            bs = new bitset<size> ();
        }
        else
            throw string("BFON::(): wrong type");

    }

void BitsFileOfNodes::WriteNodeToFile (Node* n)  //writes offset, length, next (this order)
    {
        if (type != 'w')
            throw string("BFON::WriteNodeToFile(): wrong type");
        int i;
        //bool was_flushed = false;
        unsigned short mask = (unsigned short) 1;

        for (i=0; i<offbitsn; i++)     //writing offset
        {
            if (curr+i == size)

                this->Flush();

            //cout << (curr+i) % size << ( (n->offset & mask) != 0 ? 1 : 0 ) << endl;
            (*bs)[ (curr+i) % size ] = ( (n->offset & mask) != 0 ? 1 : 0 );


            mask <<=1;
        }

        curr = (curr + i - 1) % size + 1;
        //cout << bs->to_string() << endl;
        mask = (unsigned short) 1;

        for (i=0; i<lenbitsn; i++)     //writing length
        {
            if (curr+i == size)

                this->Flush();


            (*bs)[ (curr+i) % size ] = ((n->length & mask) != 0 ? 1 : 0);
            mask<<=1;
        }

        curr = (curr + i - 1) % size + 1;
        //cout << bs->to_string() << endl;
        mask = (unsigned short) 1;

        for (i=0; i<8; i++)     //writing next
        {
            if (curr+i == size)

                this->Flush();


            (*bs)[ (curr+i) % size ] = ((n->next & mask) != 0 ? 1 : 0);
            mask <<= 1;
        }

        curr = (curr + i - 1) % size + 1;
        ++n_of_nodes;
        //cout << bs->to_string() << endl;
    }

Node* BitsFileOfNodes::ReadNodeFromFile()
    {
        if ( type != 'r' )
            throw string("BFON::ReadNodeFromFile(): wrong type");
        if ( nodes_red == n_of_nodes )  // check if cannot read more
            return nullptr;


        Node* n = new Node;
        n->offset = n->length = n->next = 0;
        int i;

        //unsigned short mask = (unsigned short) 1;
        for (i = 0; i < offbitsn; i++)  //reading offset
        {
            if (curr+i == size)
                this->Fill();
            n->offset |= (*bs)[ (curr+i) % size ] << i;

        }
        curr = (curr + i - 1) % size + 1;

        for (i = 0; i < lenbitsn; i++)  //reading length
        {
            if (curr+i == size)
                this->Fill();
            n->length |= (*bs)[ (curr+i) % size ] << i;

        }
        curr = (curr + i - 1) % size + 1;

        for (i = 0; i < 8; i++)  //reading next
        {
            if (curr+i == size)
                this->Fill();
            n->next |= (*bs)[ (curr+i) % size ] << i;
        }
        curr = (curr + i - 1) % size + 1;


        if ( ++nodes_red == n_of_nodes )
            eof = true;

        //cout << "RNFF:" << n->offset << ' ' << n->length << ' ' << n->next << endl;

        return n;

    }

void BitsFileOfNodes::Flush ()
    {
        if (type != 'w')
            throw string("BFON::Flush(): wrong type");

        //cout << "Flush:" << bs->to_string() << endl;

        unsigned long long n = bs->to_ullong();
        outp->write((char*)&n, sizeof(unsigned long long));
    }

void BitsFileOfNodes::Close ()
    {
        if ( type == 'w' )
        {
            this->Flush();
            outp->seekp( 0 );
            outp->write( (char *) &n_of_nodes, sizeof(unsigned long long) );
            outp->close();
        }
        else if (type == 'r')
        {
            inp->close();
            delete bs;
        }
        else
            throw string("BFON::Flush(): wrong type");
    }

void BitsFileOfNodes::Fill()
    {
        if (type != 'r')
            throw string("BFON::Fill(): wrong type");

        unsigned long long n;
        inp->read( (char*)&n, sizeof(unsigned long long) );
        if (bs != nullptr)
            delete bs;
        bs = new bitset<size>(n);
        //cout << "Fill :" << bs->to_string() << endl;
    }

bool BitsFileOfNodes::Eof()
    {
        return eof;
    }


int lz77Compress (const string& inpath, const string& outpath, char type)
{
    int winsize, dictsize;
    unsigned char bitsn;


    if (type == '1')
    {
        winsize = 5*1024;
        dictsize = 4*1024;
        bitsn = (unsigned char) 13;

    }
    else if (type == '2')
    {
        winsize = 10*1024;
        dictsize = 8*1024;
        bitsn = (unsigned char) 14;
    }
    else if (type == '3')
    {
        winsize = 20*1024;
        dictsize = 16*1024;
        bitsn = (unsigned char) 15;
    }
    else throw "lz77Compress::wrong type";


    int j,bytes;
    int ah_size = winsize-dictsize;   // size of ahead buffer
    char ch,unmatched;

    auto stTime = chrono::high_resolution_clock::now();   // for time measuring

    ifstream inp(inpath, ifstream::in | ifstream::binary );     // inp file
    //cout << '!' << endl;
    if ( !inp.is_open() )
    {
        cerr << "Error opening file";
        return 1;
    }

    //ofstream outp(outpath, ofstream::out | ofstream::binary | ofstream::trunc);     // outp file
    BitsFileOfNodes outf (outpath, 'w', bitsn, bitsn);

    CycledCharArray* dict = new CycledCharArray(dictsize);
    CycledCharArray* ah_buffer = new CycledCharArray(ah_size);    // ahead buffer


    // first of all, we need to fill ahead buffer
    bytes=0;
    while ( !inp.eof() && !ah_buffer->IsFull() )
    {
        inp.get(ch);
        ah_buffer->addValueToTail(ch);
        ++bytes;
    }


    //  loop with reading from file and creating nodes
    Node n;
    while ( ah_buffer->getLength() > 0 )    // ??? >3
    {
        n = dict->findMatch( ah_buffer );

        unmatched = ah_buffer->getValue( n.length );
        //need to add to dict all elements, not only unmatched
        for (j=0; j<=n.length; j++)     // will add every matching and one unmatched symbol to the end of dict
        {
            dict->addValueToTail( ah_buffer->getValue(0) );
            if ( !inp.eof() )
            {
                inp.get(ch);

                ah_buffer->Push(ch);
                ++bytes;
            }
            else
                ah_buffer->removeFirst();

        }

        n.next = unmatched;
        outf.WriteNodeToFile( &n );

        /*
        ah_buffer->printArr();
        sleep(1);
         */

    }

    outf.Close();
    cout << "BytesCompressed:" << bytes << endl;

    auto endTime = chrono::high_resolution_clock::now();
    long long int resTime = (endTime - stTime).count();

    cout << "Compression Finished. ";
    cout << "Time of compression = " << resTime << endl;

}

int lz77Decompress (const string& inpath, const string& outpath, char type)
{
    int winsize, dictsize;
    unsigned char bitsn;
    if (type == '1')
    {
        winsize = 5*1024;
        dictsize = 4*1024;
        bitsn = (unsigned char)13;
    }
    else if (type == '2')
    {
        winsize = 10*1024;
        dictsize = 8*1024;
        bitsn = (unsigned char)14;
    }
    else if (type == '3')
    {
        winsize = 20*1024;
        dictsize = 16*1024;
        bitsn = (unsigned char)15;
    }
    else throw "lz77Decompress::wrong type";

    auto stTime = chrono::high_resolution_clock::now();   // for time measuring

    BitsFileOfNodes inpf(inpath, 'r', bitsn, bitsn);
    CycledCharArray* dict = new CycledCharArray(dictsize);
    Node* n;
    char ch;

    int nodesnumber = 0;

    ofstream outf(outpath, ofstream::out | ofstream::binary | ofstream::trunc );    // output file

    while (!inpf.Eof())
    {
        n = inpf.ReadNodeFromFile();
        for (int i=0; i<n->length; i++)
        {
            ch = dict->getValue( dict->getLength() - n->offset );
            dict->addValueToTail( ch );
            outf.write( &ch, sizeof (char) );
        }

        dict->addValueToTail( n->next );
        outf.write( &( n->next ), sizeof (char) );

    ++nodesnumber;

    }



    inpf.Close();

    outf.close();

    auto endTime = chrono::high_resolution_clock::now();
    long long int resTime = (endTime - stTime).count();

    cout << "Decompression Finished. ";
    cout << "Time of decompression = " << resTime << endl;

    delete dict;

}

/*
 *
 */




int main(int argc, char* argv[])    //filename, mode[-c,-d], type [-1,-2,-3]
{

    try {
        if (argc != 4)
        {
            cout << "Wrong arguments. Format: filename [-c/-d] [-1/-2/-3]" << endl << "-c = compression" << endl << "-d = decompression"
                 << endl << "-1 = winsize 5 dictsize 4" << endl << "-2 = winsize 10 dictsize 8" << endl << "-3 = winsize 20 dictsize 16" << endl;
            return 1;
        }

        string filename(argv[1]);
        string type(argv[3]);

        if (argv[2][1] == 'c')
            lz77Compress(filename, filename+".lz77", type[1] );

        if (argv[2][1] == 'd')
            lz77Decompress(filename, filename+".unlz77", type[1] );


    }
    catch (string str)
    {
        cout << str;
    }

    return 0;
}



