#include <iostream>
#include <iomanip>
#include <windows.h>
#include <winioctl.h>
#include <vector>

// Define a struct to represent a disk sector
class DiskSector {
private:
    wchar_t* _diskPath;    // Disk path
    int _sectorNum;        // Sector number
    DWORD _bytesRead;      // Number of bytes read
    char* _buffer;         // Sector data

public:
    // Constructor
    DiskSector(const wchar_t* path, int sector) : _diskPath(const_cast<wchar_t*>(path)), _sectorNum(sector), _bytesRead(0) {
        _buffer = new char[getSectorSize()];
        this->readDiskSector();
    }

    // Destructor
    ~DiskSector() {
        delete[] _buffer;
    }

    // Function to get the sector size
    DWORD getSectorSize() {
        HANDLE hDisk = CreateFileW(_diskPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDisk == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Unable to open disk." << std::endl;
            return 0;
        }

        DWORD bytesReturned = 0;
        DISK_GEOMETRY_EX diskGeometry;

        if (!DeviceIoControl(hDisk, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeometry, sizeof(diskGeometry), &bytesReturned, NULL)) {
            std::cerr << "Error: Unable to retrieve disk geometry." << std::endl;
            CloseHandle(hDisk);
            return 0;
        }

        CloseHandle(hDisk);
        return diskGeometry.Geometry.BytesPerSector;
    }

    // Function to read a disk sector and store it in DiskSector object
    void readDiskSector() {
        HANDLE hDisk = CreateFileW(this->_diskPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDisk == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Unable to open disk. Check permission!" << std::endl;
            return;
        }

        LARGE_INTEGER li{};
        li.QuadPart = this->_sectorNum * this->getSectorSize();
        SetFilePointerEx(hDisk, li, NULL, FILE_BEGIN);

        if (!ReadFile(hDisk, this->_buffer, this->getSectorSize(), &this->_bytesRead, NULL)) {
            std::cerr << "Error: Unable to read disk." << std::endl;
        }

        CloseHandle(hDisk);
    }

    // LITTLE ENDIAN
    void readSectorLE(int offset, int size) {
        if (offset < 0 || offset + size > this->getSectorSize()) {
            std::cerr << "Error: Invalid offset and size." << std::endl;
            return;
        }

        char* data = new char[size];
        for (int i = offset + size - 1, j = 0; i >= offset; i--, j++) {
            data[j] = static_cast<char>(this->_buffer[i]);
        }

        if (size == 1) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[0] & 0xFF) << std::endl;
        }
        else {
            for (int i = 0; i < size; ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i] & 0xFF) << " ";
            }
            std::cout << std::endl;
        }

        delete[] data;
    }

    // BIG ENDIAN
    void readSectorBE(int offset, int size) {
        if (offset < 0 || offset + size > this->getSectorSize()) {
            std::cerr << "Error: Invalid offset and size." << std::endl;
            return;
        }

        char* data = new char[size];
        for (int i = offset, j = 0; i < offset + size; i++, j++) {
            data[j] = static_cast<char>(this->_buffer[i]);
        }

        if (size == 1) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[0] & 0xFF) << std::endl;
        }
        else {
            for (int i = 0; i < size; ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i] & 0xFF) << " ";
            }
            std::cout << std::endl;
        }

        delete[] data;
    }

    void printSector() {
        for (int i = 0; i < this->getSectorSize(); i++) {
			if (i % 16 == 0) std::cout << std::endl;
			std::cout << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)(unsigned char)this->_buffer[i] << " ";
			std::cout << std::dec << std::setfill(' ');
		}
        std::cout << std::endl;
	}

};

//Read NTFS MBR
void readNTFS(const wchar_t* diskPath) {
	//Create a DiskSector object for sector 0 of the disk
	DiskSector sector(diskPath, 0);
    sector.printSector();

    //Read entry 1 of the partition table
    std::cout << "Partition 1: " << std::endl;
    sector.readSectorBE(0x1BE, 16);

    //Read entry 2 of the partition table
    std::cout << "Partition 2: " << std::endl;
    sector.readSectorBE(0x1CE, 16);

    //Read entry 3 of the partition table
    std::cout << "Partition 3: " << std::endl;
    sector.readSectorBE(0x1DE, 16);

    //Read entry 4 of the partition table
    std::cout << "Partition 4: " << std::endl;
    sector.readSectorBE(0x1EE, 16);
}

//Explain the boot sector
void explainBootSector(const wchar_t* diskPath) {
    //Create a DiskSector object for sector 0 of the disk
    DiskSector sector(diskPath, 0);

    std::cout << "Bytes per sector: ";
    sector.readSectorLE(0x0B, 2);
    std::cout << "Sectors per cluster: ";
    sector.readSectorLE(0x0D, 1);
    std::cout << "Reserved sectors: ";
    sector.readSectorLE(0x0E, 2);
    std::cout << "Number of FATs: ";
    sector.readSectorLE(0x10, 1);
    std::cout << "Root entries: ";
    sector.readSectorLE(0x11, 2);
    std::cout << "Total sectors: ";
    sector.readSectorLE(0x13, 2);
    std::cout << "Media type: ";
    sector.readSectorLE(0x15, 1);
    std::cout << "Sectors per FAT: ";
    sector.readSectorLE(0x16, 2);
    std::cout << "Sectors per track: ";
    sector.readSectorLE(0x18, 2);
    std::cout << "Number of heads: ";
    sector.readSectorLE(0x1A, 2);
    std::cout << "Hidden sectors: ";
    sector.readSectorLE(0x1C, 4);
    std::cout << "Total sectors: ";
    sector.readSectorLE(0x20, 4);
    std::cout << "Drive number: ";
    sector.readSectorLE(0x24, 1);
    std::cout << "Signature: ";
    sector.readSectorLE(0x26, 1);
    std::cout << "Volume ID: ";
    sector.readSectorLE(0x27, 4);
    std::cout << "Volume label: ";
    sector.readSectorBE(0x2B, 11);
    std::cout << "File system type: ";
    sector.readSectorLE(0x36, 8);
    std::cout << "Bootable signature: ";
    sector.readSectorLE(0x1FE, 2);
}

int main() {
    const wchar_t* diskPath = L"\\\\.\\PhysicalDrive3"; // Provide the path to your physical disk

    readNTFS(diskPath);

    system("pause");

    return 0;
}