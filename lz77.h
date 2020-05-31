// КДЗ по дисциплине Алгоритмы и структуры данных 2017-2018 уч.год
// Коршиков Федор Андреевич, дата (08.04.2018)
// Среда разработки: CLion, состав проекта: lz77.cpp lz77.h
// Что сделано, а что нет: реализован только алгоритм lz77

#ifndef LZ77_MAIN_H
#define LZ77_MAIN_H

using namespace std;

struct Node
{
    short offset;
    short length;
    char next;
};

class CycledCharArray   //array of fixed max size, designed for using like buffer
{
public:
    CycledCharArray(int s) ; //s - size (max size)

    ~CycledCharArray();

    char getValue(int i);


    void addValueToTail(char val);   // if arr is not full, writes after last el. if full, tail sets to the first el. returns value to be deleted


    void Push(char val);

    void removeFirst();


    int getLength ();


    bool IsFull();


    void printArr ();    // for testing

    Node findMatch(CycledCharArray* source) ;    // returns node with filled length and offset. next is not defined. if match wasnt found, offset = length = 0


private:
    char* arr;
    int size;
    unsigned short curr_length;
    bool is_full;
    int start;
    int end;   // element to write in. if arr is not empty, first uninitialized el

};

class BitsFileOfNodes     //using std::bitset for buffering. when buffer is full, writes it to file
{

public:
    BitsFileOfNodes (string path, char t, unsigned char offb, unsigned char lenb);     //needs to know how much bytes take every field (next is 8bit)


    void WriteNodeToFile (Node* n) ; //writes offset, length, next (this order);


    Node* ReadNodeFromFile();


    void Flush ();


    void Close ();


    void Fill();


    bool Eof();


private:

    unsigned long long n_of_nodes;
    unsigned long long nodes_red;
    bool eof;
    unsigned char offbitsn;     // how many bits does Node.offset take
    unsigned char lenbitsn;     // how many bits does Node.length take
    int curr;                   // position to write a bit
    char type;
    ifstream* inp;
    ofstream* outp;
    static const int size = numeric_limits<unsigned long long>::digits;
    bitset<size>* bs;
};

int lz77Compress (const string& inpath, const string& outpath, char type);


int lz77Decompress (const string& inpath, const string& outpath, char type);



#endif //LZ77_MAIN_H
