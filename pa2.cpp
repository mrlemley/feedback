/* 
 * File:   pa2.cpp
 * Author: brettski
 *
 * Created on October 25, 2014, 3:32 PM
 */

//#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>

using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::setw;
using std::ceil;

/*
 * This program emulates an operating system's allocation of memory to programs.
 * More specifically, it simulates management of a simple page-based memory sys-
 * tem with 32, 4KB long, pages of contiguous memory.
 * 
 * Users are able to add and kill programs using a simple CLI. Once memory is
 * released by a program it becomes available to new programs.
 * 
 * This implementation uses doubly linked lists to track the pages and their
 * contents.
 */

// pageBlock represents some number of contiguous pages
// pageBlock can be "free" or contain a program

struct pageBlock {
    int size;
    string contents;
    pageBlock* next;
    pageBlock* prev;
    //MRL: since only PageMemory should be able to change the contents of pageBlock attributes. Consider making attributes read only. 
    //MRL: PageMemory could be a friend class.  The other option would be to have PageMemory only return copies of pageBlock so that 
    //MRL: anyone messing with a pageBlock will not adversely affect the behavior of PageMemory.
    
};

// PageMemory is the main memory class
// it contains all pages and functions necessary to manage them

class PageMemory {
protected: //MRL: I'd go with private unless a clear need is identified. 
    char algorithm; //MRL: since algorithms are known, an enum type would be better here. It provides documentation and avoids users providing an unknown value.
    int pageSizeKB; //MRL: consider using unsigned integers since the value of pageSizeKB and pageTotal cannot be negative. This comment pretty much applies to all int values in this algorithm.
    int pageTotal;
    pageBlock* pageHead;
    pageBlock* pageN;
    pageBlock* pageTemp;
    //string pageMap[];
public:

    //MRL: public functions are described for developers using your class. You should describe what the function does as well as the inputs and outputs. 
    //MRL: any error returns (throws or constant return values) should be explained so the developer knows what to expect.
    PageMemory(char);
    pageBlock* findSpace(int);
    pageBlock* findProg(string);
    int addProg(int, string);
    int killProg(string);
    int fragments();
    void genMap();
};

// PageMemory Constructor initializes with one 32 page block of free memory

PageMemory::PageMemory(char algo) {
    pageSizeKB = 4;
    pageTotal = 32;
    algorithm = algo;
    pageHead = new pageBlock;
    pageHead->size = pageTotal;
    pageHead->contents = "Free"; //MRL: a constant should be used for "Free".
    pageHead->prev = NULL;
    pageHead->next = NULL;
    pageTemp = NULL;
    pageN = NULL;
}

// returns a free block of memory which is at least as large as size
// returns the largest available block in "worst case" mode
// returns the smallest available block in "best case" mode

pageBlock* PageMemory::findSpace(int size) {
    pageTemp = pageHead;
    pageBlock* pageFit = NULL;
    //Best-Fit: finds the smallest possible block of free memory
    if (algorithm == 'B') {
        while (pageTemp != NULL) {
            if (pageFit == NULL) {
                if (pageTemp->size >= size && pageTemp->contents == "Free") {
                    pageFit = pageTemp;
                }
            } else if (pageTemp->size >= size && pageTemp->size < pageFit->size
                    && pageTemp->contents == "Free") {
                pageFit = pageTemp;
            }
            pageTemp = pageTemp->next;
        }

    }
    //Worst-Fit: finds the largest possible block of free memory
    if (algorithm == 'W') {
        while (pageTemp != NULL) {
            if (pageFit == NULL) {
                if (pageTemp->size >= size) {
                    pageFit = pageTemp;
                }
            } else if (pageTemp->size >= size && pageTemp->size > pageFit->size) {
                pageFit = pageTemp;
            }
            pageTemp = pageTemp->next;
        }
    }
    pageTemp = NULL;
    return pageFit;
};

// adds a program into free memory

int PageMemory::addProg(int sizeKB, string name) {
    //MRL: what if user calls "addProg(0, "") - more error handling needed.
    //MRL: what about "addProg(maxint +1, "")
    //MRL: what about "addProg(1, "Free")
    //MRL: what about "addProg(-1, "oops")
    
    int size = ceil((sizeKB * 1.0) / pageSizeKB);
    if (findProg(name) != NULL) return -1; //Program already running 
      //MRL: constants should be provided for the -1 and 0 return values. 
    pageTemp = findSpace(size);
    if (pageTemp == NULL) return 0; //Not enough memory available
    // program needs exact number of pages as are contained in the free block
    if (pageTemp->size == size) {
        pageTemp->contents = name;
        // free block is trimmed to make space for the program
    } else {
        pageN = new pageBlock;
        pageN->size = (pageTemp->size) - size;
        pageN->contents = "Free";
        pageN->prev = pageTemp;
        pageN->next = pageTemp->next;

        pageTemp->size = size;
        pageTemp->contents = name;
        pageTemp->next = pageN;
    }

    pageN = NULL;
    pageTemp = NULL;
    return size;
}

// returns the block associated with a program called name
pageBlock* PageMemory::findProg(string name) {
    pageBlock* location = pageHead;
    while (location != NULL) {
        if (location->contents == name) {
            return location;
        };
        location = location->next;
    };
    return NULL; // no such program exists
}

// removes a program from memory and restores vacant memory to free
int PageMemory::killProg(string name) {
    //MRL: what about killProg("Free") will it cause any problems?
    
    pageTemp = findProg(name);
    if (pageTemp == NULL) return 0; //Program could not be found
    int freed = pageTemp->size;
    pageTemp->contents = "Free";
    // add newly freed memory to free memory before program
    if (pageTemp->prev != NULL && (pageTemp->prev)->contents == "Free") {
        pageN = pageTemp->prev;
        pageN->size += (pageTemp->size);
        pageN->next = pageTemp->next;
        delete pageTemp;
        pageTemp = pageN;
    };
    // add newly freed memory to free memory after program
    if (pageTemp->next != NULL && (pageTemp->next)->contents == "Free") {
        pageN = pageTemp;
        pageTemp = pageN->next;
        pageN->size += (pageTemp->size);
        pageN->next = pageTemp->next;
        delete pageTemp;
    }
    pageTemp = NULL;
    pageN = NULL;
    return freed;
}

// returns the number of non-contiguous blocks of free memory
int PageMemory::fragments() {
    int fragments = 0;
    pageTemp = pageHead;
    while (pageTemp != NULL) {
        if (pageTemp->contents == "Free") {
            fragments++;
        }
        pageTemp = pageTemp->next;
    }
    return fragments;
}

// prints the memory map with programs and free space labeled
void PageMemory::genMap() {
    int displayWidth = 8;
    string pageMap[pageTotal];
    pageTemp = pageHead;
    int index = 0;
    while (pageTemp != NULL) {
        for (int page = 0; page < pageTemp->size; page++, index++) {
            pageMap[index] = pageTemp->contents;
        }
        pageTemp = pageTemp->next;
    }
    index = 0;
    while (index < pageTotal) {
        for (int col = 0; col < displayWidth; col++) {
            cout << setw(5) << pageMap[index];
            index++;
        }
        cout << endl;
    }
}

int main(int argc, char* argv[]) {
    char algo = 'B';
    bool done = false;
    int input = 0;
    string name;
    int size;
    int output;

    if (argc >= 2) {
        if (argv[1][0] == 'w') {
            algo = 'W';
            cout << "Using worst best algorithm" << endl << endl;
        } else {
            cout << "Using best fit algorithm" << endl << endl;
        }
    } else {
        cout << "Using best fit algorithm" << endl << endl;
    }

    PageMemory pM = PageMemory(algo);

    cout << "1. Add program" << endl
            << "2. Kill program" << endl
            << "3. Fragmentation" << endl
            << "4. Print Memory" << endl
            << "5. Exit" << endl << endl;

    while (!done) {
        cout << "choice - ";
        cin >> input;

        switch (input) {
            case 1: //add program
                cout << "Program name - ";
                cin >> name;
                cout << "Program size (KB) - ";
                cin >> size;
                output = pM.addProg(size, name);
                if (output == -1) {
                    cout << "Error, Program " << name << " is already running.";
                } else if (output == 0) {
                    cout << "Error, not enough memory for Program " << name;
                } else {
                    cout << "Program " << name << " added successfully, "
                            << output << " page(s) used.";
                }
                break;
            case 2: //kill program
                cout << "Program name - ";
                cin >> name;
                output = pM.killProg(name);
                if (output == 0) {
                    cout << "Program " << name << " could not be found in memory";
                } else {
                    cout << "Program " << name << " successfully killed, "
                            << output << " page(s) reclaimed.";
                }
                break;
            case 3: //report fragments
                cout << "There are " << pM.fragments() << " fragment(s).";
                break;
            case 4: //print memory map
                pM.genMap();
                break;
            case 5:
                done = true;
                break;
        }
        cout << endl << endl;
    }


    return 0;
}

//MRL: where are the unit tests? Maybe that's outside of the scope of the project. But in real life, you wouldn't write code without them.
