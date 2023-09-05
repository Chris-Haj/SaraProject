#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 512
int BLOCKS_IN_USE;


char  decToBinary(int n , char &c)
{
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0) {
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--) {
        if (binaryNum[j]==1)
            c = c | 1u << j;
    }
}

// #define SYS_CALL
// ============================================================================
class fsInode {
    int fileSize;
    int block_in_use;

    int directBlock1;
    int directBlock2;
    int directBlock3;
public:
    void setDirectBlock1(int directBlock1) {
        fsInode::directBlock1 = directBlock1;
    }

    void setDirectBlock2(int directBlock2) {
        fsInode::directBlock2 = directBlock2;
    }

    void setDirectBlock3(int directBlock3) {
        fsInode::directBlock3 = directBlock3;
    }

private:

    int singleInDirect;
    int doubleInDirect;
    int block_size;
public:

    int getFileSize() const {
        return fileSize;
    }

    void setFileSize(int fileSize) {
        fsInode::fileSize = fileSize;
    }

    int getBlockInUse() const {
        return block_in_use;
    }

    void setBlockInUse(int blockInUse) {
        block_in_use = blockInUse;
    }

    int getSingleInDirect() const {
        return singleInDirect;
    }

    void setSingleInDirect(int singleInDirect) {
        fsInode::singleInDirect = singleInDirect;
    }

    int getBlockSize() const {
        return block_size;
    }

    void setBlockSize(int blockSize) {
        block_size = blockSize;
    }

    int getDirect1(){
        return directBlock1;
    }
    int getDirect2(){
        return directBlock2;
    }
    int getDirect3(){
        return directBlock3;
    }

public:
    fsInode(int _block_size) {
        fileSize = 0;
        block_in_use = 0;
        block_size = _block_size;
        directBlock1 = -1;
        directBlock2 = -1;
        directBlock3 = -1;
        singleInDirect = -1;
        doubleInDirect = -1;

    }

    // YOUR CODE......


};

// ============================================================================
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse;

public:
    FileDescriptor(string FileName, fsInode* fsi) {
        file.first = FileName;
        file.second = fsi;
        inUse = true;
    }

    // ------------------------------------------------------------------------
    string getFileName() {
        return file.first;
    }

    fsInode* getInode() {

        return file.second;

    }


    bool isInUse() {
        return (inUse);
    }
    void setInUse(bool _inUse) {
        inUse = _inUse ;
    }


};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk {
    FILE *sim_disk_fd;

    bool is_formated; //Disk has to be formatted (=1) to start any operation

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir ;
    map<string, FileDescriptor> NamedDir;
    // OpenFileDescriptors --  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    map<int ,FileDescriptor > OpenFileDescriptors;
    /*
     * FileDescriptor
 *      pair<string, fsInode*> file;
        bool inUse;
     * */

    int direct_enteris;
    int block_size;

public:
    // ------------------------------------------------------------------------
    fsDisk() {
        sim_disk_fd = fopen( DISK_SIM_FILE , "r+" );
        assert(sim_disk_fd);
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
        is_formated = false;

    }

    int FindEmptyDescriptor(){
        if (OpenFileDescriptors.empty())
            return 0;
        int emptyIndex = 0;
        for (auto it = OpenFileDescriptors.begin(); it != OpenFileDescriptors.end(); it++) {
            if (it->first == emptyIndex)
                emptyIndex++;
            else
                return emptyIndex;
        }
        return emptyIndex;
    }

    ~fsDisk(){
        delete[] BitVector;
        MainDir.clear();
        OpenFileDescriptors.clear();
        fclose(sim_disk_fd);
    }

    // ------------------------------------------------------------------------
    void listAll() {
        int i = 0;
        for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
            cout << "index: " << i << ": FileName: " << it->second.getFileName() <<  " , isInUse: " << it->second.isInUse() << " file Size: " << it->second.getInode()->getFileSize();
            i++;
        }
        char bufy;
        cout << "Disk content: '" ;
        for (i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
            cout << bufy;
        }
        cout << "'" << endl;
    }

    // ------------------------------------------------------------------------
    void fsFormat(int blockSize = 4) {
        if(is_formated){
            MainDir.clear();
            OpenFileDescriptors.clear();
        }
        BitVectorSize = DISK_SIZE/blockSize;
        block_size = blockSize;
        BitVector = new int[BitVectorSize];
        BLOCKS_IN_USE = 0;
    }

    int OpenFile(string FileName){
        if (is_formated == false || MainDir.find(FileName)!= MainDir.end())
            return -1;
        FileDescriptor curfile = NamedDir.find(FileName)->second;
        if (curfile.isInUse()== true)
            return -1;
        int newOpenIndex = FindEmptyDescriptor();
        curfile.setInUse(true);
        OpenFileDescriptors.insert({newOpenIndex,curfile});
        return newOpenIndex;
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {
        if (is_formated == false || MainDir.find(fileName)!= MainDir.end())
            return -1;
        fsInode newFile(block_size);
        FileDescriptor newFileDesc(fileName, &newFile);
        int indexDescriptor = FindEmptyDescriptor();
        OpenFileDescriptors.insert({indexDescriptor,newFileDesc});
        MainDir.insert({fileName,&newFile});
        NamedDir.insert({fileName,newFileDesc});
        return indexDescriptor;
    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {
        if (is_formated == false)
            return "-1";
        if (OpenFileDescriptors.count(fd) <= 0)
            return "-1";
        string fileName = OpenFileDescriptors.find(fd)->second.getFileName();
        OpenFileDescriptors.find(fd)->second.setInUse(false);
        OpenFileDescriptors.erase(fd);
        return fileName;
    }
    // ------------------------------------------------------------------------

//    @TODO
    int WriteToFile(int fd, char *buf, int len ) {

        if (is_formated == false) {
            return -1;
        }

        if (OpenFileDescriptors.find(fd) == OpenFileDescriptors.end()) { // If File is not open
            return -1;
        }
        //If file is open, find it.
        FileDescriptor FileD = OpenFileDescriptors.find(fd)->second;
        fsInode *fileNode = FileD.getInode();
        int fileSize = fileNode->getFileSize();

        //If file is empty, and has no blocks,locate an empty block and start writing.
        if(fileSize == 0){
            /*1- calculate how many blocks needed to write string
             * 2-If not enough space in file and in disk, continue,
             * else remove extra chars.
             * 3- Find empty block, write into it, repeat, until no more space or finished writing string.*/
            int neededBlocks = ceil(len/block_size);
            if (neededBlocks > 3){// 1 is the single indirect block
                neededBlocks+=1;
            }
            else if(neededBlocks > 3 + block_size){ // 1+blocksize is the the single indirect block + the double indirect block with its single indirect blocks.
                neededBlocks+=2+block_size;
            }



        }
        //If file is not empty, go to current block that has space and continue writing.
        else{

        }



    }
    // @TODO
    int DelFile( string FileName ) {

    }
    // @TODO
    int ReadFromFile(int fd, char *buf, int len ) {

    }

    // @TODO
    int GetFileSize(int fd) {
        if(is_formated == false || OpenFileDescriptors.find(fd) == OpenFileDescriptors.end())
            return -1;
        FileDescriptor file = OpenFileDescriptors.find(fd)->second;
        return file.getInode()->getFileSize();
    }

    // ------------------------------------------------------------------------
    int CopyFile(string srcFileName, string destFileName) {}

    // ------------------------------------------------------------------------
    int MoveFile(string srcFileName, string destFileName) {}

    // ------------------------------------------------------------------------
    int RenameFile(string oldFileName, string newFileName) {
        if (is_formated == false || MainDir.find(oldFileName) == MainDir.end())
            return -1;
        FileDescriptor file = NamedDir.find(oldFileName)->second;
        fsInode *fsInode = MainDir.find(oldFileName)->second;
        if (file.isInUse() == true)
            return -1;
        MainDir.erase(oldFileName);
        MainDir.insert({newFileName,fsInode});
        NamedDir.erase(oldFileName);
        NamedDir.insert({newFileName,file});
        return 0;
    }

};

int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    string fileName2;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
                delete fs;
                exit(0);
                break;

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                cin >> direct_entries;
                fs->fsFormat(blockSize);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 9:   // copy file
                cin >> fileName;
                cin >> fileName2;
                fs->CopyFile(fileName, fileName2);
                break;

            case 10:  // rename file
                cin >> fileName;
                cin >> fileName2;
                fs->RenameFile(fileName, fileName2);
                break;

            default:
                break;
        }
    }

}