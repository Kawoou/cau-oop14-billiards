#ifndef __APROJECT_SECUFS__
#define __APROJECT_SECUFS__

#pragma once

class CASecuFS
{
private:
	bool						isInitLibrary;

public:
	CREATE_VOLUME				CreateVolume;
	OPEN_VOLUME					OpenVolume;
	CLOSE_VOLUME				CloseVolume;
	DESTORY_VOLUME				DestoryVolume;
	GET_VOLUME_INFORMATION		GetVolumeInformation;
	GET_VOLUME_TOTAL_SPACE		GetVolumeTotalSpace;
	GET_VOLUME_USING_SPACE		GetVolumeUsingSpace;
	GET_VOLUME_FREE_SPACE		GetVolumeFreeSpace;
	CREATE_RECOVERY_KEY			CreateRecoveryKey;
	CREATE_RECOVERY_KEY_EX		CreateRecoveryKeyEx;
	CHANGE_PASSWORD				ChangePassword;
	RECOVERY_PASSWORD			RecoveryPassword;
	CHECK_RECOVERY_KEY			CheckRecoveryKey;
	CREATE_DIRECTORY			CreateDirectory;
	CREATE_DIRECTORY_EX			CreateDirectoryEx;
	PRESENCE_DIRECTORY			PresenceDirectory;
	REMOVE_DIRECTORY			RemoveDirectory;
	REMOVE_DIRECTORY_EX			RemoveDirectoryEx;
	RENAME_DIRECTORY			RenameDirectory;
	RENAME_DIRECTORY_EX			RenameDirectoryEx;
	IMPORT_FILE					ImportFile;
	IMPORT_FILE_EX				ImportFileEx;
	EXPORT_FILE					ExportFile;
	EXPORT_FILE_EX				ExportFileEx;
	DELETE_FILE					DeleteFile;
	DELETE_FILE_EX				DeleteFileEx;
	RENAME_FILE					RenameFile;
	RENAME_FILE_EX				RenameFileEx;
	FIND_FIRST_FILE				FindFirstFile;
	FIND_NEXT_FILE				FindNextFile;
	FIND_CLOSE					FindClose;
	OPEN_FILE					OpenFile;
	OPEN_FILE_EX				OpenFileEx;
	READ_FILE					ReadFile;
	WRITE_FILE					WriteFile;
	SEEK_FILE					SeekFile;
	GET_FILE_OFFSET				GetFileOffset;
	GET_FILE_EOF				GetFileEOF;
	CLOSE_FILE					CloseFile;

public:
	static CASecuFS* GetSingleton (void);

public:
    CASecuFS (void);
	~CASecuFS (void);

public:
	bool InitLibrary (char* lpLibFileName);
};

#endif