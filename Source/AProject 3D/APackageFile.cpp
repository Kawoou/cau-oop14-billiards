#include "APFinclude.h"

CASecuFS* CASecuFS::GetSingleton (void)
{
	static CASecuFS pRet;

	return &pRet;
}

CASecuFS::CASecuFS (void)
{
	isInitLibrary = false;

	CreateVolume = NULL;
	OpenVolume = NULL;
	CloseVolume = NULL;
	DestoryVolume = NULL;
	GetVolumeInformation = NULL;
	GetVolumeTotalSpace = NULL;
	GetVolumeUsingSpace = NULL;
	GetVolumeFreeSpace = NULL;
	CreateRecoveryKey = NULL;
	CreateRecoveryKeyEx = NULL;
	ChangePassword = NULL;
	RecoveryPassword = NULL;
	CheckRecoveryKey = NULL;
	CreateDirectory = NULL;
	CreateDirectoryEx = NULL;
	PresenceDirectory = NULL;
	RemoveDirectory = NULL;
	RemoveDirectoryEx = NULL;
	RenameDirectory = NULL;
	RenameDirectoryEx = NULL;
	ImportFile = NULL;
	ImportFileEx = NULL;
	ExportFile = NULL;
	ExportFileEx = NULL;
	DeleteFile = NULL;
	DeleteFileEx = NULL;
	RenameFile = NULL;
	RenameFileEx = NULL;
	FindFirstFile = NULL;
	FindNextFile = NULL;
	FindClose = NULL;
	OpenFile = NULL;
	OpenFileEx = NULL;
	ReadFile = NULL;
	WriteFile = NULL;
	SeekFile = NULL;
	GetFileOffset = NULL;
	GetFileEOF = NULL;
	CloseFile = NULL;

#if defined (DEBUG) | defined (_DEBUG)
	InitLibrary ("./SecuFS.Debug.dll");
#else
	InitLibrary ("./SecuFS.Release.dll");
#endif
}

CASecuFS::~CASecuFS (void)
{
}

bool CASecuFS::InitLibrary (char* lpLibFileName)
{
	if (isInitLibrary)
		return isInitLibrary;

	HINSTANCE hInst = LoadLibraryA (lpLibFileName);
	if (hInst)
	{
		isInitLibrary = true;
		CreateVolume = (CREATE_VOLUME)GetProcAddress (hInst, "SecuFS_CreateVolume");
		OpenVolume = (OPEN_VOLUME)GetProcAddress (hInst, "SecuFS_OpenVolume");
		CloseVolume = (CLOSE_VOLUME)GetProcAddress (hInst, "SecuFS_CloseVolume");
		DestoryVolume = (DESTORY_VOLUME)GetProcAddress (hInst, "SecuFS_DestoryVolume");
		GetVolumeInformation = (GET_VOLUME_INFORMATION)GetProcAddress (hInst, "SecuFS_GetVolumeInformation");
		GetVolumeTotalSpace = (GET_VOLUME_TOTAL_SPACE)GetProcAddress (hInst, "SecuFS_GetVolumeTotalSpace");
		GetVolumeUsingSpace = (GET_VOLUME_USING_SPACE)GetProcAddress (hInst, "SecuFS_GetVolumeUsingSpace");
		GetVolumeFreeSpace = (GET_VOLUME_FREE_SPACE)GetProcAddress (hInst, "SecuFS_GetVolumeFreeSpace");
		CreateRecoveryKey = (CREATE_RECOVERY_KEY)GetProcAddress (hInst, "SecuFS_CreateRecoveryKey");
		CreateRecoveryKeyEx = (CREATE_RECOVERY_KEY_EX)GetProcAddress (hInst, "SecuFS_CreateRecoveryKeyEx");
		ChangePassword = (CHANGE_PASSWORD)GetProcAddress (hInst, "SecuFS_ChangePassword");
		RecoveryPassword = (RECOVERY_PASSWORD)GetProcAddress (hInst, "SecuFS_RecoveryPassword");
		CheckRecoveryKey = (CHECK_RECOVERY_KEY)GetProcAddress (hInst, "SecuFS_CheckRecoveryKey");
		CreateDirectory = (CREATE_DIRECTORY)GetProcAddress (hInst, "SecuFS_CreateDirectory");
		CreateDirectoryEx = (CREATE_DIRECTORY_EX)GetProcAddress (hInst, "SecuFS_CreateDirectoryEx");
		PresenceDirectory = (PRESENCE_DIRECTORY)GetProcAddress (hInst, "SecuFS_PresenceDirectory");
		RemoveDirectory = (REMOVE_DIRECTORY)GetProcAddress (hInst, "SecuFS_RemoveDirectory");
		RemoveDirectoryEx = (REMOVE_DIRECTORY_EX)GetProcAddress (hInst, "SecuFS_RemoveDirectoryEx");
		RenameDirectory = (RENAME_DIRECTORY)GetProcAddress (hInst, "SecuFS_RenameDirectory");
		RenameDirectoryEx = (RENAME_DIRECTORY_EX)GetProcAddress (hInst, "SecuFS_RenameDirectoryEx");
		ImportFile = (IMPORT_FILE)GetProcAddress (hInst, "SecuFS_ImportFile");
		ImportFileEx = (IMPORT_FILE_EX)GetProcAddress (hInst, "SecuFS_ImportFileEx");
		ExportFile = (EXPORT_FILE)GetProcAddress (hInst, "SecuFS_ExportFile");
		ExportFileEx = (EXPORT_FILE_EX)GetProcAddress (hInst, "SecuFS_ExportFileEx");
		DeleteFile = (DELETE_FILE)GetProcAddress (hInst, "SecuFS_DeleteFile");
		DeleteFileEx = (DELETE_FILE_EX)GetProcAddress (hInst, "SecuFS_DeleteFileEx");
		RenameFile = (RENAME_FILE)GetProcAddress (hInst, "SecuFS_RenameFile");
		RenameFileEx = (RENAME_FILE_EX)GetProcAddress (hInst, "SecuFS_RenameFileEx");
		FindFirstFile = (FIND_FIRST_FILE)GetProcAddress (hInst, "SecuFS_FindFirstFile");
		FindNextFile = (FIND_NEXT_FILE)GetProcAddress (hInst, "SecuFS_FindNextFile");
		FindClose = (FIND_CLOSE)GetProcAddress (hInst, "SecuFS_FindClose");
		OpenFile = (OPEN_FILE)GetProcAddress (hInst, "SecuFS_OpenFile");
		OpenFileEx = (OPEN_FILE_EX)GetProcAddress (hInst, "SecuFS_OpenFileEx");
		ReadFile = (READ_FILE)GetProcAddress (hInst, "SecuFS_ReadFile");
		WriteFile = (WRITE_FILE)GetProcAddress (hInst, "SecuFS_WriteFile");
		SeekFile = (SEEK_FILE)GetProcAddress (hInst, "SecuFS_SeekFile");
		GetFileOffset = (GET_FILE_OFFSET)GetProcAddress (hInst, "SecuFS_GetFileOffset");
		GetFileEOF = (GET_FILE_EOF)GetProcAddress (hInst, "SecuFS_GetFileEOF");
		CloseFile = (CLOSE_FILE)GetProcAddress (hInst, "SecuFS_CloseFile");
	}

	return isInitLibrary;
}