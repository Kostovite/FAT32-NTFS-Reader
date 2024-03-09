#include "DiskReader.h"

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

//Read GPT
void readGPT(const wchar_t* diskPath) {
	//Create a DiskSector object for sector 1 of the disk
	DiskSector sector(diskPath, 1);
	sector.printSector();
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

std::vector<short> scanMBRPhysicalDrives() {
    std::vector<short> driveID;

    for (int driveNumber = 0; driveNumber < 16; ++driveNumber) { // Assuming there are at most 16 physical drives
        std::wstring drivePath = L"\\\\.\\PhysicalDrive" + std::to_wstring(driveNumber);

        HANDLE hDisk = CreateFileW(drivePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

        // If the handle is valid, and the disk is an MBR disk, add it to the list
        if (hDisk != INVALID_HANDLE_VALUE) {
            if (!isGPT(drivePath.c_str())) {
				driveID.push_back(driveNumber);
			}
			CloseHandle(hDisk);
		}
        else {
			break;
		}
    }

    return driveID;
}

std::vector<sectorData> scanPartitions(const wchar_t* diskPath)
{
    std::vector<sectorData> partitions;

	//Create a DiskSector object for sector 0 of the disk
	DiskSector sector(diskPath, 0);

	for (int ind = 0; ind < 4; ind++) {
		sectorData partitionEntry = sector.saveSectorBE(0x1BE + 0x10 * ind, 16);
		
        //If the partition type is 0, it means the partition is not used
        if (partitionEntry[4] == 0x00) {
            partitions.push_back(partitionEntry);
        }
	}

	return partitions;
}

//Check if the disk is a GPT disk
bool isGPT(const wchar_t* diskPath) {
	DiskSector sector(diskPath, 1);
	sectorData signature = sector.saveSectorBE(0x00, 8);

    if (signature == sectorData({ 0x45, 0x46, 0x49, 0x20, 0x50, 0x41, 0x52, 0x54 })) {
		return true;
	}

	return false;
}

int main() {
    std::vector<short> drives = scanMBRPhysicalDrives();

    std::wcout << L"Found " << drives.size() << L" MBR physical drive(s)." << std::endl;
    for (short id : drives) {
        std::wcout << L"Physical drive ID: " << id << std::endl;
    }

    //Scan the partitions of all the drives found
    for (short id : drives) {
		std::wstring drivePath = L"\\\\.\\PhysicalDrive" + std::to_wstring(id);
		std::vector<sectorData> partitions = scanPartitions(drivePath.c_str());

		std::wcout << L"Found " << partitions.size() << L" partition(s) on drive " << id << std::endl;
        for (sectorData partition : partitions) {
			partition.printVector(sectorData::hex);
		}
	}

    return 0;
}