#include <iostream>
#include <cstring>
#include <functional>
#include <map>
#include <fstream>

#define BYTES_PER_SECTOR 512
#define NUMBER_OF_FAT_ENTRIES 256
#define NUMBER_OF_ROOT_DIR_SECTORS 2

#define NUMBER_OF_FILE_SECTORS 252
#define NUMBER_OF_RESERVED_SECTORS 4

#define FAT_FREE_SECTOR 0x00
#define FAT_RESERVED_SECTOR 0x01

#define ROOT_FREE_ENTRY 0x0

std::string getDiskImageName(char** args) {
    return std::string(args[2]);
}

std::string getInputFileName(char **args) {
    return std::string(args[3]);
}

char* fixFileName(std::string filename) {
    int startPos = 0;
    for(int i = filename.size()-1; i >= 0; --i) {
        if(filename[i] == '\\' || filename[i] == '/') {
            startPos = i+1;
            break;
        }
    }

    // If the filename string represents a folder, do bad stuff
    if(startPos >= filename.size()) {
        return NULL;
    }

    char* fixed = new char[7];
    for(size_t i = 0; i < 7 && i < filename.size(); ++i) {
        fixed[i] = filename[startPos + i];
    }

    return fixed;
}

unsigned int getNumFreeFATEntries(FILE* diskFile) {
    // Seek to the beginning of the FAT entries in the disk image
    fseek(diskFile, 512, 0);

    bool foundFree = false;
    unsigned int numEntries = 0;
    for(int i=0;i<NUMBER_OF_FAT_ENTRIES;++i) {
        char entry = fgetc(diskFile);

        if(!foundFree) {
            if(entry == FAT_FREE_SECTOR) {
                foundFree = true;
            }
        } else {
            numEntries += 1;
        }
    }

    return numEntries;
}

// Adds a FAT entry to the disk image
//
// returns false if no free entry was found
// return true if entry was added
// (This function is not responsible for closing the file pointer)
bool addFATEntry(FILE* diskFile, unsigned char sector) {
    // Seek to the beginning of the FAT entries in the disk image
    fseek(diskFile, 512, 0);

    unsigned int numEntries = 0;
    for(int i=0;i<NUMBER_OF_FAT_ENTRIES;++i) {
        char entry = fgetc(diskFile);

        if(entry == FAT_FREE_SECTOR) {
            // Seek back one byte to add entry
            fseek(diskFile, 512+i, 0);

            // Write FAT entry
            fputc(sector, diskFile);

            return true;
        }
    }

    return false;
}

bool getFATEntryExists(FILE *diskFile, const unsigned char sector) {
    // Seek to the beginning of the FAT entries in the disk image
    fseek(diskFile, 512, 0);

    for(int i=0;i<NUMBER_OF_FAT_ENTRIES;++i) {
        unsigned char entry = fgetc(diskFile);

        if(entry == sector) {
            return true;
        }
    }

    return false;
}

void writeFileToSector(FILE *diskImage, std::ifstream &toCopyFile, const unsigned char sector) {
    // Seek to the beginning of the file
    //toCopyFile.seekg(0, std::ios_base::beg);

    // Seek to where we are writing the sector
    fseek(diskImage, sector*BYTES_PER_SECTOR, 0);

    for(int i=0;i<BYTES_PER_SECTOR;++i) {
        char data = toCopyFile.get();

        if(toCopyFile.fail()) {
            break;
        }

        fputc(data, diskImage);
    }
}

// Adds an entry to the root directory
//
// returns false if no more entries are available
// returns true if and entry was added
// (This function is not responsible for closing the file pointer)
bool addRootEntry(FILE *diskFile, std::string fileName, const unsigned char sector) {
    // Seek to the beginning of the root entries in the disk image
    fseek(diskFile, 512*2, 0);

    unsigned int numEntries = 0;
    for(int i=0;i<NUMBER_OF_ROOT_DIR_SECTORS*BYTES_PER_SECTOR;++i) {
        // No entry can be valid and contain a zero, so to check if an
        // entry is available, we check if a byte is zero
        if(fgetc(diskFile) == ROOT_FREE_ENTRY) {
            // Seek back one byte to add entry
            fseek(diskFile, 512*2 + i, 0);

            // Write root entry filename
            char* fixedFileName = fixFileName(fileName);
            std::cout << "Writing filename '" << fixedFileName << "' to root...";

            for(size_t i = 0; i < 7;++i)
                fputc(fixedFileName[i], diskFile);

            delete fixedFileName;
            fixedFileName = NULL;

            // Write root entry sector
            fputc(sector, diskFile);

            return true;
        }
    }

    return false;
}

// Copies a file to a disk image
//
// returns false if there is not enough room on disk
// returns true if succeeded
// (This function is not responsible for closing the file pointer)
bool copyFileToDisk(FILE *diskFile, std::ifstream &toCopyFile, std::string toCopyFileName) {
    // First we need to check if there is enough room on disk
    // by looking at how many free entries there are in the FAT.
    // This means we also need to take the size of the input file
    // and round it up to the nearest 512. The number of FAT entries
    // needed is the rounded file size/512.

    // Then we need to find file sectors that are available
    // For the first file sector (sector 8 [we start at 1]) we check
    // if it is in the FAT. If it isn't, go to the next sector and check.
    // If it is, mark the sector number in the FAT, update the root directory,
    // and write the first 512 bytes of the file to that sector. Then rinse & repeat
    // for the rest of the file

    long begin = toCopyFile.tellg();
    toCopyFile.seekg(0, std::ios_base::end);
    long end = toCopyFile.tellg();

    const long fileSize = end - begin;

    // Round up the number of file sectors needed
    const auto numFileSectors = (fileSize < 512) ? 1 : ((fileSize%512 == 0) ? fileSize/512 : (fileSize/512)+1);

    std::cout << "Attempting to copy " << fileSize << " bytes to disk image (" << numFileSectors << " sectors)...";

    // Check free FAT entries
    unsigned int numEntries = 0;
    if((numEntries = getNumFreeFATEntries(diskFile)) < numFileSectors) {
        std::cerr << "Not enough space on disk image only " << numEntries << " entries left\n";
        return false;
    }

    // Seek to the beginning of the file for writing
    toCopyFile.seekg(0, std::ios_base::beg);

    unsigned int numSectorsWritten = 0;

    bool addedRootEntry = false;
    for(unsigned int i=0;i<NUMBER_OF_FILE_SECTORS && numSectorsWritten<numFileSectors;++i) {
        const auto currentSector = (unsigned char)(i+NUMBER_OF_RESERVED_SECTORS);

        // Check if the sector we are looking at is part
        // of another file
        if(getFATEntryExists(diskFile, currentSector)) {
            continue;
        } else {

            if(!addedRootEntry) {
                // Add entry to the root directory
                addRootEntry(diskFile, toCopyFileName, currentSector);
                addedRootEntry = true;
            }

            addFATEntry(diskFile, currentSector);

            writeFileToSector(diskFile, toCopyFile, currentSector);
            numSectorsWritten += 1;
        }
    }

    return true;
}

// Formats a disk image so that we can begin writing files
// (This function is not responsible for closing the file stream)
bool formatDiskImage(std::ofstream &file) {
    // Write File Allocation Table
    // The table is an array of 8bit entries
    // pointing to the next sector on the disk image.
    // 8bits only allows for 256 sectors, 4 of
    // which are taken up by the boot sector,
    // the FAT itself (FAT 1 byte per entry, 256 entries per sector = 1 sector),
    // and the Root Directory (8bits for filename, 8bits for first sector of file) 2 sectors.
    // Which means there are only 252 sectors available

    // Zero out boot sector
    for(int i=0;i<BYTES_PER_SECTOR;++i) {
        file << (uint8_t)0;
    }

    // Initialize FAT
    for(int i=0;i<NUMBER_OF_FAT_ENTRIES;++i) {
        if(i < NUMBER_OF_RESERVED_SECTORS) {
            file << (uint8_t)FAT_RESERVED_SECTOR;
            continue;
        }

        file << (uint8_t)FAT_FREE_SECTOR;
    }

    // Initialize Root Directory
    for(int i=0;i<NUMBER_OF_ROOT_DIR_SECTORS*BYTES_PER_SECTOR;++i) {
        file << (uint8_t)0;
    }

    // Initialize file sectors
    for(int i=0;i<NUMBER_OF_FILE_SECTORS*BYTES_PER_SECTOR;++i) {
        file << (uint8_t)0;
    }

    return true;
}

bool MakeDiskCommand(int argc, char **args) {
    if(argc != 3) {
        std::cerr << "Invalid arguments\n";
    }

    std::string diskName = getDiskImageName(args);
    std::ofstream file(diskName, std::ios::binary);

    if(file.is_open()) {
        formatDiskImage(file);

        file.close();
        std::cout << "Created disk image '" << diskName << "'\n";
        return 0;
    } else {
          std::cerr << "Failed to create disk image '" << diskName << "'\n";
    }
}

bool FormatDiskCommand(int argc, char **args) {
    if(argc != 2) {
        std::cerr << "Invalid arguments\n";
    }

    std::cout << "make disk command\n";
}

bool RemoveFileFromDiskCommand(int argc, char **args) {
    if(argc != 2) {
        std::cerr << "Invalid arguments\n";
    }

    std::cout << "make disk command\n";
}

bool CopyFileToDiskCommand(int argc, char **args) {
    if(argc != 4) {
        std::cerr << "Invalid arguments\n";
    }

    // Open up the given file to copy
    std::string toCopyFileName = getInputFileName(args);
    std::ifstream toCopyFile(toCopyFileName, std::ios::binary);

    // Check if it managed to open
    if(toCopyFile.is_open()) {

        //Open up the given disk image file
        std::string diskName = getDiskImageName(args);
        FILE* file = fopen(diskName.c_str(), "r+");

        // Check if the disk image managed to open
        if (file != NULL) {
            if(!copyFileToDisk(file, toCopyFile, toCopyFileName)) {
                std::cerr << "Failed to copy file to disk image\n";
            } else {
                std::cout << "Copied file '" << toCopyFileName << "' to '" << diskName << "'\n";
            }

            fclose(file);
            return 0;
        } else {
            std::cerr << "Failed to open disk image '" << diskName << " for copying'\n";
        }

        toCopyFile.close();
    } else {
        std::cerr << "Could not open file '" << toCopyFileName << "'\n";
    }
}

bool CopyBootloaderToDiskCommand(int argc, char **args) {
    if(argc != 4) {
        std::cerr << "Invalid arguments\n";
    }

    // Open up the given bootloader file
    std::string bootloaderName = getInputFileName(args);
    std::ifstream bootloaderFile(bootloaderName, std::ios::binary);

    // Check if it managed to open
    if(bootloaderFile.is_open()) {

        //Open up the given disk image file
        std::string diskName = getDiskImageName(args);
        FILE* file = fopen(diskName.c_str(), "r+");

        // Check if the disk image managed to open
        if (file != NULL) {
            for(int i=0;i<BYTES_PER_SECTOR;++i) {
                char data = 0;
                bootloaderFile.read(&data, sizeof(char));

                fputc(data, file);
            }

            fclose(file);

            std::cout << "Copied bootloader to boot sector of '" << diskName << "'\n";
            return 0;
        } else {
            std::cerr << "Failed to copy bootloader to disk image '" << diskName << "'\n";
        }

        bootloaderFile.close();
    } else {
        std::cerr << "Could not open bootloader file '" << bootloaderName << "'\n";
    }
}

#define commandSignature std::function<bool(int argc, char **args)>

typedef std::map<std::string, commandSignature> commandDictionary;
typedef std::pair<std::string, commandSignature> command;

const commandDictionary cmds = {
        command("mk", MakeDiskCommand),   // Make new disk image
        command("fmt", FormatDiskCommand),  // Format disk image
        command("rm", RemoveFileFromDiskCommand),   // Remove file from disk image
        command("cp", CopyFileToDiskCommand),   // Copy file to disk image
        command("bootcp", CopyBootloaderToDiskCommand),   // Copy file to disk image
};

int main(int argc, char **args) {

    if (argc < 2) {
        std::cout << "Please specify a command\n";
        return 1;
    }

    // Check if first argument is a valid command
    auto search = cmds.find(args[1]);
    if (search != cmds.end()) {
        // Call function relate to command
        search->second(argc, args);
    }else {
        std::cerr << "Invalid command '" << args[1] << "'\n";
        return 1;
    }

    return 0;
}