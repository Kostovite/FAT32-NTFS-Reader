﻿#include <iostream>
#include <iomanip>
#include <windows.h>
#include <winioctl.h>
#include <vector>

/// <summary>
/// Class to represent part of the data of a disk sector
/// </summary>
class sectorData
{
private:
    std::vector<char> _data;

public:
    //Enum for hex, dec, char
    enum dataPresentation
    {
        hex = 16,
        dec = 10,
        chr = 256
    };

    //Constructor
    sectorData(const std::vector<char>& data) : _data(data) {}

    //Decstructor
    ~sectorData() {}

    //Operator = to copy data
    sectorData& operator = (const sectorData& other)
    {
        if (this != &other)
        {
			this->_data = other._data;
		}
		return *this;
	}

    sectorData& operator = (const std::vector<char>& data)
    {
		this->_data = data;
		return *this;
	}

    //Convert to vector
    std::vector<char> toVector() const
    {
        return this->_data;
    }

    void printVector(dataPresentation presentation) {
        switch (presentation) {
        case hex:
            for (const char& c : this->_data) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c & 0xFF) << " ";
            }
            break;
        case dec:
        {
            unsigned long int dec = 0;
            for (const char& c : this->_data) {
                dec = (dec << 8) + (c & 0xFF);
            }
            std::cout << std::dec << dec;
        }
        break;
        case chr:
            for (const char& c : this->_data) {
                if (isprint(c)) {
                    std::cout << c;
                }
                else {
                    std::cout << "."; // Print a dot for non-printable characters
                }
            }
            break;
        default:
            throw std::runtime_error("Error: Invalid data presentation type.");
        }

        std::cout << std::endl;
    }
};

// Define a struct to represent a disk sector
/// <summary>
/// Class to store a disk sector
/// </summary>
class DiskSector {
private:
    wchar_t* _diskPath;    // Disk path
    int _sectorNum;        // Sector number
    DWORD _bytesRead;      // Number of bytes read
    char* _buffer;         // Sector data

public:
    // Constructor
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="path">Path to the disk</param>
    /// <param name="sector">Sector number</param>
    DiskSector(const wchar_t* path, int sector) : _diskPath(const_cast<wchar_t*>(path)), _sectorNum(sector), _bytesRead(0) {
        _buffer = new char[getSectorSize()];
        this->readDiskSector();
    }

    // Destructor
    ~DiskSector() {
        delete[] _buffer;
        _buffer = nullptr;
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
            throw std::runtime_error("Error: Unable to read disk.");
        }

        CloseHandle(hDisk);
    }

    // LITTLE ENDIAN
    sectorData saveSectorLE(int offset, int size) {
        std::vector<char> data;

        if (offset < 0 || offset + size > this->getSectorSize()) {
            throw std::runtime_error("Error: Invalid offset and size.");
        }

        data.reserve(size);

        for (int i = offset + size - 1; i >= offset; i--) {
            data.push_back(static_cast<char>(this->_buffer[i]));
        }

        sectorData sectorData = data;

        return sectorData;
    }

    // BIG ENDIAN
    sectorData saveSectorBE(int offset, int size) {
		std::vector<char> data;

        if (offset < 0 || offset + size > this->getSectorSize()) {
			throw std::runtime_error("Error: Invalid offset and size.");
		}

		data.reserve(size);

        for (int i = offset; i < offset + size; i++) {
			data.push_back(static_cast<char>(this->_buffer[i]));
		}

		sectorData sectorData = data;

		return sectorData;
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

//Explain partition entry
void explainPartitionEntry(DiskSector &entry) {

    for (int ind = 0; ind < 4; ind++) {
        std::cout << "Partition " << ind + 1 << " entry: ";
        entry.saveSectorBE(0x1BE + 0x10 * ind, 16).printVector(sectorData::hex);

		std::cout << "Status: ";
		entry.saveSectorLE(0x1BE + 0x10 * ind, 1).printVector(sectorData::hex);

		std::cout << "First sector CHS: ";
		entry.saveSectorBE(0x1BE + 0x02 + 0x10 * ind, 3).printVector(sectorData::hex);

        std::cout << "Last sector CHS: ";
        entry.saveSectorBE(0x1BE + 0x05 + 0x10 * ind, 3).printVector(sectorData::hex);

		std::cout << "Partition type: ";
		entry.saveSectorLE(0x1BE + 0x04 + 0x10 * ind, 1).printVector(sectorData::hex);

		std::cout << "First sector LBA: ";
		entry.saveSectorLE(0x1BE + 0x08 + 0x10 * ind, 4).printVector(sectorData::dec);

		std::cout << "Number of sectors: ";
		entry.saveSectorLE(0x1BE + 0x0C + 0x10 * ind, 4).printVector(sectorData::dec);
	}
}

//Read MBR
void readMBR(const wchar_t* diskPath) {
	//Create a DiskSector object for sector 0 of the disk
	DiskSector sector(diskPath, 0);
    sector.printSector();

    explainPartitionEntry(sector);
}

//Explain the boot sector
void explainBootSector(const wchar_t* diskPath) {
    //Create a DiskSector object for sector 0 of the disk
    DiskSector sector(diskPath, 0);

    std::cout << "Bytes per sector: ";
    sector.saveSectorLE(0x0B, 2).printVector(sectorData::dec);

    std::cout << "Sectors per cluster: ";
    sector.saveSectorLE(0x0D, 1).printVector(sectorData::dec);

    std::cout << "Reserved sectors: ";
    sector.saveSectorLE(0x0E, 2).printVector(sectorData::dec);

    std::cout << "Number of FATs: ";
    sector.saveSectorLE(0x10, 1).printVector(sectorData::dec);

    std::cout << "Root entries: ";
    sector.saveSectorLE(0x11, 2).printVector(sectorData::dec);

    std::cout << "Total sectors: ";
    sector.saveSectorLE(0x13, 2).printVector(sectorData::dec);

    std::cout << "Media type: ";
    sector.saveSectorLE(0x15, 1).printVector(sectorData::hex);

    std::cout << "Sectors per FAT: ";
    sector.saveSectorLE(0x16, 2).printVector(sectorData::dec);

    std::cout << "Sectors per track: ";
    sector.saveSectorLE(0x18, 2).printVector(sectorData::dec);

    std::cout << "Number of heads: ";
    sector.saveSectorLE(0x1A, 2).printVector(sectorData::dec);

    std::cout << "Hidden sectors: ";
    sector.saveSectorLE(0x1C, 4).printVector(sectorData::dec);

    std::cout << "Total sectors: ";
    sector.saveSectorLE(0x20, 4).printVector(sectorData::dec);

    std::cout << "Drive number: ";
    sector.saveSectorLE(0x24, 1).printVector(sectorData::hex);

    std::cout << "Signature: ";
    sector.saveSectorLE(0x26, 1).printVector(sectorData::hex);

    std::cout << "Volume ID: ";
    sector.saveSectorLE(0x27, 4).printVector(sectorData::hex);

    std::cout << "Volume label: ";
    sector.saveSectorBE(0x2B, 11).printVector(sectorData::chr);

    std::cout << "File system type: ";
    sector.saveSectorBE(0x36, 8).printVector(sectorData::chr);

    std::cout << "Bootable signature: ";
    sector.saveSectorLE(0x1FE, 2).printVector(sectorData::hex);
}

int main() {
    const wchar_t driveName[] = L"\\\\.\\PhysicalDrive3";
    readMBR(driveName);
}