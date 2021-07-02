#include "debugmenu.h"

HMODULE dllModule;

CControllerConfigManager *ctrldummy = nil;

/*
 * TGA reader
 */

#pragma pack(push, 1)
struct TGAHeader
{
	RwInt8  IDlen;
	RwInt8  colorMapType;
	RwInt8  imageType;
	RwInt16 colorMapOrigin;
	RwInt16 colorMapLength;
	RwInt8  colorMapDepth;
	RwInt16 xOrigin, yOrigin;
	RwInt16 width, height;
	RwUInt8 depth;
	RwUInt8 descriptor;
};
#pragma pack(pop)

RwImage*
readTGA(uint8 *p)
{
	RwImage *image;

	TGAHeader header;
	int depth = 0, palDepth = 0;
	memcpy(&header, p, sizeof(header));
	p += sizeof(header);
	
	assert(header.imageType == 1 || header.imageType == 2);
	p += header.IDlen;
	if(header.colorMapType){
		assert(header.colorMapOrigin == 0);
		depth = (header.colorMapLength <= 16) ? 4 : 8;
		palDepth = header.colorMapDepth;
		assert(palDepth == 24 || palDepth == 32);
	}else{
		depth = header.depth;
		assert(depth == 24 || depth == 32);
	}

	image = RwImageCreate(header.width, header.height, depth == 24 ? 32 : depth);
	RwImageAllocatePixels(image);
	RwRGBA *palette = header.colorMapType ? image->palette : NULL;
	if(palette){
		int maxlen = depth == 4 ? 16 : 256;
		int i;
		for(i = 0; i < header.colorMapLength; i++){
			palette[i].blue = *p++;
			palette[i].green = *p++;
			palette[i].red = *p++;
			palette[i].alpha = 0xFF;
			if(palDepth == 32)
				palette[i].alpha = *p++;
		}
		for(; i < maxlen; i++){
			palette[i].red = palette[i].green = palette[i].blue = 0;
			palette[i].alpha = 0xFF;
		}
	}

	RwUInt8 *pixels = image->cpPixels;
	if(!(header.descriptor & 0x20))
		pixels += (image->height-1)*image->stride;
	for(int y = 0; y < image->height; y++){
		RwUInt8 *line = pixels;
		for(int x = 0; x < image->width; x++){
			if(palette)
				*line++ = *p++;
			else{
				line[2] = *p++;
				line[1] = *p++;
				line[0] = *p++;
				line += 3;
				if(depth == 32)
					*line++ = *p++;
				if(depth == 24)
					*line++ = 0xFF;
			}
		}
		pixels += (header.descriptor&0x20) ?
		              image->stride : -image->stride;
	}
	return image;
}

RwImage*
readTGA(const char *filename)
{
	RwImage *img;
	FILE *file = fopen(filename, "rb");
	fseek(file, 0, 2);
	int len = ftell(file);
	uint8 *data = (uint8*)malloc(len);
	fseek(file, 0, 0);
	fread(data, 1, len, file);
	fclose(file);
	img = readTGA(data);
	free(data);
	return img;
}

RwImage*
readTGA(int res)
{
	RwImage *img;
	HRSRC resource = FindResource(dllModule, MAKEINTRESOURCE(res), RT_RCDATA);
	uint8 *data = (uint8*)LoadResource(dllModule, resource);
	img = readTGA(data);
	FreeResource(data);
	return img;
}


/*
 * Debug menu
 */

void (*Render_orig)(unsigned long);
void
Render(unsigned long t)
{
	Render_orig(t);
	((void (__cdecl *)(long))0x5F5A80)(0);
	DebugMenuRender();
	((void (__cdecl *)(long))0x5F5A80)(1);
}

void (__thiscall *Update_orig)(void *);
void __fastcall Update(void *This)
{
	DebugMenuProcess();
	Update_orig(This);
}

void
patchMH(void)
{
	InterceptCall(&Render_orig, Render, 0x5F2C1A);
	InterceptVmethod(&Update_orig, Update, 0x737168);
}

BOOL WINAPI
DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
	if(reason == DLL_PROCESS_ATTACH){
		dllModule = hInst;

		patchMH();

	}

	return TRUE;
}
