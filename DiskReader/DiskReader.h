#pragma once

#include <iostream>
#include <iomanip>
#include <windows.h>
#include <winioctl.h>
#include <vector>
#include <string>
#include <ntddscsi.h>
#include <initguid.h>

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

    std::string getVector(dataPresentation presentation) {
		std::string result;
        switch (presentation) {
		case hex:
            for (const char& c : this->_data) {
				result += std::to_string(static_cast<int>(c & 0xFF)) + " ";
			}
			break;
		case dec:
        {
			unsigned long int dec = 0;
            for (const char& c : this->_data) {
				dec = (dec << 8) + (c & 0xFF);
			}
			result = std::to_string(dec);
		}
		break;
		case chr:
            for (const char& c : this->_data) {
                if (isprint(c)) {
					result += c;
				}
                else {
					result += "."; // Print a dot for non-printable characters
				}
			}
			break;
		default:
			throw std::runtime_error("Error: Invalid data presentation type.");
		}

		return result;
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

    std::vector<char> getSector() {
		std::vector<char> sector;
		sector.reserve(this->getSectorSize());
        for (int i = 0; i < this->getSectorSize(); i++) {
			sector.push_back(this->_buffer[i]);
		}
		return sector;
	}
};


//Experimental functions

void explainPartitionEntry(DiskSector& entry);

void readMBR(const wchar_t* diskPath);

void explainBootSector(const wchar_t* diskPath);

void readGPT(const wchar_t* diskPath);

std::vector<short> scanPhysicalDrives();