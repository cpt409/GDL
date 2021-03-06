
#include "StdAfx.h"
#include "WeenieLandCoverage.h"


void SaveBitmapFile(const char *szFile, UINT Width, UINT Height, UINT Bpp, LPVOID lpData)
{
	// Bpp = Bits per Pixel
	UINT BytesPerPixel = Bpp >> 3;

	// Create the output file
	HANDLE hFile = CreateFile(szFile, GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// DEBUGOUT("Couldn't create output bitmap file: \"%s\"!\r\n", szFile);
		return;
	}

	// Create the Bitmap Header
	BITMAPFILEHEADER FileHeader;
	BITMAPINFOHEADER InfoHeader;

	// Scan lines must be DWORD-aligned.
	UINT LineSize = Width * BytesPerPixel;
	UINT LinePadding = 0;

	if (LineSize & 3)
		LinePadding += 4 - (LineSize & 3);

	FileHeader.bfType = 'MB';
	FileHeader.bfSize = sizeof(FileHeader) + sizeof(InfoHeader) + (LineSize + LinePadding) * Height;
	FileHeader.bfReserved1 = 0;
	FileHeader.bfReserved2 = 0;
	FileHeader.bfOffBits = sizeof(FileHeader) + sizeof(InfoHeader);

	InfoHeader.biSize = sizeof(InfoHeader);
	InfoHeader.biWidth = Width;
	InfoHeader.biHeight = Height;
	InfoHeader.biPlanes = 1;
	InfoHeader.biBitCount = Bpp;

	InfoHeader.biCompression = BI_RGB;
	InfoHeader.biSizeImage = FileHeader.bfSize;
	InfoHeader.biXPelsPerMeter = 0;
	InfoHeader.biYPelsPerMeter = 0;
	InfoHeader.biClrUsed = 0;
	InfoHeader.biClrImportant = 0;

	// Write the Bitmap File Header.
	DWORD Dummy;

	if (!WriteFile(hFile, &FileHeader, sizeof(FileHeader), &Dummy, NULL))
	{
		//DEBUGOUT("Failed writing bitmap file header!\r\n");
	}

	if (!WriteFile(hFile, &InfoHeader, sizeof(InfoHeader), &Dummy, NULL))
	{
		//DEBUGOUT("Failed writing bitmap info header!\r\n");
	}

	// Bitmap's prefer bottom-up format, so we're doing it that way.
	for (long y = (long)Height - 1; y >= 0; y--)
	{
		WriteFile(hFile, (BYTE *)lpData + y*LineSize, LineSize, &Dummy, NULL);

		DWORD Padding = 0;
		WriteFile(hFile, &Padding, LinePadding, &Dummy, NULL);
	}

	CloseHandle(hFile);
}

void WeenieLandCoverage::SaveCoverageBitmap()
{
	BYTE *imageData = new BYTE[256 * 8 * 256 * 8 * 3];
	for (DWORD y = 0; y < (256 * 8); y++)
	{
		for (DWORD x = 0; x < (256 * 8); x++)
		{
			BYTE set = 0;

			DWORD landcell_id = 0;

			landcell_id |= 0xFF000000 & ((x >> 3) << 24);
			landcell_id |= 0x00FF0000 & ((y >> 3) << 16);
			landcell_id |= ((x & 7) << 3);
			landcell_id |= ((y & 7) << 0);

			if (_creatureSpawns.find(landcell_id) != _creatureSpawns.end())
				set = 0xFF;

			BYTE *output = &imageData[((y * 256 * 8) + x) * 3];
			output[0] = set;
			output[1] = set;
			output[2] = set;
		}
	}

	SaveBitmapFile("c:\\coverage_map.bmp", 256 * 8, 256 * 8, 24, imageData);
	delete [] imageData;
}

WeenieLandCoverage::WeenieLandCoverage()
{
}

WeenieLandCoverage::~WeenieLandCoverage()
{
	Reset();
}

void WeenieLandCoverage::Reset()
{
	_creatureSpawns.clear();
}

void WeenieLandCoverage::Initialize()
{
#ifndef QUICKSTART
	LOG(Data, Normal, "Loading weenie spawns...\n");
	LoadLocalStorage();
	LOG(Data, Normal, "Loaded %d cells with spawns...\n", _creatureSpawns.size());

	DWORD totalSpawns = 0;
	for (auto spawn : _creatureSpawns)
	{
		totalSpawns += spawn.second.num_used;
	}

	LOG(Data, Normal, "Loaded %d spawns within them...\n", totalSpawns);
#endif
}

void WeenieLandCoverage::LoadLocalStorage()
{
	BYTE *data = NULL;
	DWORD length = 0;
	if (LoadDataFromFile("data\\weenie\\landspawns\\generated.bin", &data, &length))
	{
		BinaryReader reader(data, length);

		DWORD num = reader.Read<DWORD>();
		for (DWORD i = 0; i < num; i++)
		{
			DWORD cell_id = reader.Read<DWORD>();
			_creatureSpawns[cell_id].UnPack(&reader);
		}

		delete [] data;
	}

	SaveCoverageBitmap();
}

SmartArray<DWORD> *WeenieLandCoverage::GetSpawnsForCell(DWORD cell_id)
{
	return (SmartArray<DWORD> *)_creatureSpawns.lookup(cell_id);
}


