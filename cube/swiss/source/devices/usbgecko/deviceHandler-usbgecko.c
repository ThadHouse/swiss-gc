/* deviceHandler-USBGecko.c
	- device implementation for USBGecko
	by emu_kidid
 */

#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <ogc/machine/processor.h>
#include "deviceHandler.h"
#include "gui/FrameBufferMagic.h"
#include "gui/IPLFontWrite.h"
#include "swiss.h"
#include "main.h"
#include "usbgecko.h"
#include "patcher.h"

file_handle initial_USBGecko =
	{ "./",     // directory
	  0ULL,     // fileBase (u64)
	  0,        // offset
	  0,        // size
	  IS_DIR,
	  0,
	  0
	};

device_info initial_USBGecko_info = {
	0,
	0
};

extern char *getRelativeName(char *str);
	
device_info* deviceHandler_USBGecko_info(file_handle* file) {
	return &initial_USBGecko_info;
}
	
s32 deviceHandler_USBGecko_readDir(file_handle* ffile, file_handle** dir, u32 type){	
  
	// Set everything up to read
	s32 num_entries = 0, i = 0;
	file_handle *entry = NULL;
	if(strlen(ffile->name)!=1) {
		i = num_entries = 1;
		*dir = malloc( num_entries * sizeof(file_handle) );
		memset(*dir,0,sizeof(file_handle) * num_entries);
		(*dir)[0].fileAttrib = IS_SPECIAL;
		strcpy((*dir)[0].name, "..");
	}
	
	uiDrawObj_t *msgBox = DrawPublish(DrawProgressBar(true, 0, "Reading directory"));
	// Read each entry of the directory
	s32 res = usbgecko_open_dir(&ffile->name[0]);
	if(!res) return -1;
	while( (entry = usbgecko_get_entry()) != NULL ){
		if(entry->fileAttrib == IS_FILE) {
			if(!checkExtension(entry->name)) continue;
		}		
		// Make sure we have room for this one
		if(i == num_entries) {
			++num_entries;
			*dir = realloc( *dir, num_entries * sizeof(file_handle) ); 
		}
		memset(&(*dir)[i], 0, sizeof(file_handle));
		sprintf((*dir)[i].name, "%s", entry->name);
		(*dir)[i].size			= entry->size;
		(*dir)[i].fileAttrib	= entry->fileAttrib;
		++i;
	}
	DrawDispose(msgBox);
	return num_entries;
}

s32 deviceHandler_USBGecko_seekFile(file_handle* file, s32 where, s32 type){
	if(type == DEVICE_HANDLER_SEEK_SET) file->offset = where;
	else if(type == DEVICE_HANDLER_SEEK_CUR) file->offset += where;
	return file->offset;
}

s32 deviceHandler_USBGecko_readFile(file_handle* file, void* buffer, u32 length){
	s32 reallength = length;
	if(file->offset + length > file->size) {
		reallength = file->size - file->offset;
	}
	if(reallength < 0) {
		return 0;
	}
  	s32 bytes_read = usbgecko_read_file(buffer, reallength, file->offset, file->name);
	if(bytes_read > 0) file->offset += bytes_read;
	
	return bytes_read;
}

s32 deviceHandler_USBGecko_writeFile(file_handle* file, void* buffer, u32 length) {	
	s32 bytes_written = usbgecko_write_file(buffer, length, file->offset, file->name);
	if(bytes_written > 0) file->offset += bytes_written;
	
	return bytes_written;
}

s32 deviceHandler_USBGecko_setupFile(file_handle* file, file_handle* file2) {
	
	// If there are 2 discs, we only allow 21 fragments per disc.
	int maxFrags = (VAR_FRAG_SIZE/12), i = 0;
	vu32 *fragList = (vu32*)VAR_FRAG_LIST;
	s32 frags = 0, totFrags = 0;
	
	memset((void*)VAR_FRAG_LIST, 0, VAR_FRAG_SIZE);

	// Check if there are any fragments in our patch location for this game
	if(devices[DEVICE_PATCHES] != NULL) {
		print_gecko("Save Patch device found\r\n");
		
		// Look for patch files, if we find some, open them and add them as fragments
		file_handle patchFile;
		char gameID[8];
		memset(&gameID, 0, 8);
		strncpy((char*)&gameID, (char*)&GCMDisk, 4);
		
		for(i = 0; i < maxFrags; i++) {
			u32 patchInfo[4];
			patchInfo[0] = 0; patchInfo[1] = 0; 
			memset(&patchFile, 0, sizeof(file_handle));
			sprintf(&patchFile.name[0], "%sswiss_patches/%s/%i",devices[DEVICE_PATCHES]->initial->name,gameID, i);
			print_gecko("Looking for file %s\r\n", &patchFile.name);
			FILINFO fno;
			if(f_stat(&patchFile.name[0], &fno) != FR_OK) {
				break;	// Patch file doesn't exist, don't bother with fragments
			}
			
			devices[DEVICE_PATCHES]->seekFile(&patchFile,fno.fsize-16,DEVICE_HANDLER_SEEK_SET);
			if((devices[DEVICE_PATCHES]->readFile(&patchFile, &patchInfo, 16) == 16) && (patchInfo[2] == SWISS_MAGIC)) {
				if(!(frags = getFragments(&patchFile, &fragList[totFrags*3], maxFrags, patchInfo[0], patchInfo[1], DEVICE_PATCHES))) {
					return 0;
				}
				totFrags+=frags;
				devices[DEVICE_PATCHES]->closeFile(&patchFile);
			}
			else {
				break;
			}
		}
		// Check for igr.dol
		memset(&patchFile, 0, sizeof(file_handle));
		sprintf(&patchFile.name[0], "%sigr.dol", devices[DEVICE_PATCHES]->initial->name);

		FILINFO fno;
		if(f_stat(&patchFile.name[0], &fno) == FR_OK) {
			print_gecko("IGR Boot DOL exists\r\n");
			if((frags = getFragments(&patchFile, &fragList[totFrags*3], maxFrags, 0x60000000, 0, DEVICE_PATCHES))) {
				totFrags+=frags;
				devices[DEVICE_PATCHES]->closeFile(&patchFile);
				*(vu32*)VAR_IGR_DOL_SIZE = fno.fsize;
			}
		}
		// Card Type
		*(vu8*)VAR_SD_SHIFT = (u8)(sdgecko_getAddressingType(devices[DEVICE_PATCHES] == &__device_sd_a ? EXI_CHANNEL_0:(devices[DEVICE_PATCHES] == &__device_sd_b ? EXI_CHANNEL_1:EXI_CHANNEL_2)) ? 9:0);
		// Copy the actual freq
		*(vu8*)VAR_EXI_FREQ = (u8)(swissSettings.exiSpeed ? EXI_SPEED32MHZ:EXI_SPEED16MHZ);
		// Device slot (0, 1 or 2)
		*(vu8*)VAR_EXI_SLOT = (u8)(devices[DEVICE_PATCHES] == &__device_sd_a ? EXI_CHANNEL_0:(devices[DEVICE_PATCHES] == &__device_sd_b ? EXI_CHANNEL_1:EXI_CHANNEL_2));
	}
	
	print_frag_list(0);
	// Disk 1 base sector
	*(vu32*)VAR_DISC_1_LBA = fragList[2];
	// Disk 2 base sector
	*(vu32*)VAR_DISC_2_LBA = fragList[2];
	// Currently selected disk base sector
	*(vu32*)VAR_CUR_DISC_LBA = fragList[2];
	return 1;
}

s32 deviceHandler_USBGecko_init(file_handle* file) {
	s32 success = 0;
	uiDrawObj_t *msgBox = DrawPublish(DrawProgressBar(true, 0, "Looking for USBGecko in Slot B"));
	if(usb_isgeckoalive(1)) {
		s32 retries = 1000;
		DrawDispose(msgBox);
		msgBox = DrawPublish(DrawProgressBar(true, 0, "Waiting for PC ..."));
		
		usb_flush(1);
		usbgecko_lock_file(0);
		// Wait for the PC and retry 1000 times
		while(!usbgecko_pc_ready() && retries) {
			VIDEO_WaitVSync();
			retries--;
		}
		success = retries > 1 ? 1 : 0;
		if(!success) {
			DrawDispose(msgBox);
			msgBox = DrawPublish(DrawMessageBox(D_INFO,"Couldn't find PC!"));
			sleep(5);
		}
	}
	DrawDispose(msgBox);
	return success;
}

s32 deviceHandler_USBGecko_deinit(file_handle* file) {
	return 0;
}

s32 deviceHandler_USBGecko_deleteFile(file_handle* file) {
	return -1;
}

s32 deviceHandler_USBGecko_closeFile(file_handle* file) {
    return 0;
}

bool deviceHandler_USBGecko_test() {
	return usb_isgeckoalive(1);
}

DEVICEHANDLER_INTERFACE __device_usbgecko = {
	DEVICE_ID_A,
	"USB Gecko",
	"USB Gecko - Slot B only",
	"Requires PC application to be up",
	{TEX_USBGECKO, 129, 80},
	FEAT_READ|FEAT_BOOT_GCM|FEAT_REPLACES_DVD_FUNCS|FEAT_ALT_READ_PATCHES|FEAT_CAN_READ_PATCHES,
	LOC_MEMCARD_SLOT_B,
	&initial_USBGecko,
	(_fn_test)&deviceHandler_USBGecko_test,
	(_fn_info)&deviceHandler_USBGecko_info,
	(_fn_init)&deviceHandler_USBGecko_init,
	(_fn_readDir)&deviceHandler_USBGecko_readDir,
	(_fn_readFile)&deviceHandler_USBGecko_readFile,
	(_fn_writeFile)NULL,
	(_fn_deleteFile)NULL,
	(_fn_seekFile)&deviceHandler_USBGecko_seekFile,
	(_fn_setupFile)&deviceHandler_USBGecko_setupFile,
	(_fn_closeFile)&deviceHandler_USBGecko_closeFile,
	(_fn_deinit)&deviceHandler_USBGecko_deinit
};
