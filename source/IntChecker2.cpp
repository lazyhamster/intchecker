// IntChecker2.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "far2/plugin.hpp"
#include "Utils.h"
#include "RegistrySettings.h"
#include "hashing.h"

PluginStartupInfo FarSInfo;
static FARSTANDARDFUNCTIONS FSF;

// Plugin settings
static bool optDetectHashFiles = true;
static bool optClearSelectionOnComplete = true;
static bool optConfirmAbort = true;
static int optDefaultAlgo = RHASH_MD5;
static wchar_t optPrefix[32] = L"check";

static rhash_ids s_SupportedAlgos[] = {RHASH_CRC32, RHASH_MD5, RHASH_SHA1, RHASH_SHA256, RHASH_SHA512, RHASH_WHIRLPOOL};

enum HashOutputTargets
{
	OT_SINGLEFILE,
	OT_SEPARATEFILES,
	OT_DISPLAY
};

struct ProgressContext
{
	wstring FileName;
	
	int64_t TotalFileSize;
	int64_t ProcessedBytes;

	int TotalFilesCount;
	int FileIndex;

	int FileProgress;
};

// --------------------------------------- Service functions -------------------------------------------------

static const wchar_t* GetLocMsg(int MsgID)
{
	return FarSInfo.GetMsg(FarSInfo.ModuleNumber, MsgID);
}

static void DisplayMessage(const wchar_t* headerText, const wchar_t* messageText, const wchar_t* errorItem, bool isError, bool isInteractive)
{
	static const wchar_t* MsgLines[3];
	MsgLines[0] = headerText;
	MsgLines[1] = messageText;
	MsgLines[2] = errorItem;

	int linesNum = (errorItem) ? 3 : 2;
	int flags = 0;
	if (isError) flags |= FMSG_WARNING;
	if (isInteractive) flags |= FMSG_MB_OK;

	FarSInfo.Message(FarSInfo.ModuleNumber, flags, NULL, MsgLines, linesNum, 0);
}

static void DisplayMessage(int headerMsgID, int textMsgID, const wchar_t* errorItem, bool isError, bool isInteractive)
{
	DisplayMessage(GetLocMsg(headerMsgID), GetLocMsg(textMsgID), errorItem, isError, isInteractive);
}

static int DlgHlp_GetSelectionState(HANDLE hDlg, int ctrlIndex)
{
	FarDialogItem *dlgItem;
	int retVal;

	dlgItem = (FarDialogItem*) malloc(FarSInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, ctrlIndex, NULL));
	FarSInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, ctrlIndex, (LONG_PTR) dlgItem);
	retVal = dlgItem->Selected;
	free(dlgItem);

	return retVal;
}

static void DlgHlp_GetEditBoxText(HANDLE hDlg, int ctrlIndex, wstring &buf)
{
	FarDialogItem *dlgItem;

	dlgItem = (FarDialogItem*) malloc(FarSInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, ctrlIndex, NULL));
	FarSInfo.SendDlgMessage(hDlg, DM_GETDLGITEM, ctrlIndex, (LONG_PTR) dlgItem);

	buf = dlgItem->PtrData;

	free(dlgItem);
}

// --------------------------------------- Local functions ---------------------------------------------------

static void LoadSettings()
{
	RegistrySettings regOpts(FarSInfo.RootKey);
	if (regOpts.Open())
	{
		//
	}
}

static void SaveSettings()
{
	RegistrySettings regOpts(FarSInfo.RootKey);
	if (regOpts.Open(true))
	{
		//
	}
}

// Returns true if file is recognized as hash list
static bool RunValidateFiles(const wchar_t* hashListPath, bool silent)
{
	//TODO: implement
	return false;
}

static bool AskForHashGenerationParams(rhash_ids &selectedAlgo, bool &recursive, HashOutputTargets &outputTarget, wstring &outputFileName)
{
	FarDialogItem DialogItems []={
		/*0*/{DI_DOUBLEBOX,		3, 1, 41,18, 0, 0, 0, 0, L"Generate"},

		/*1*/{DI_TEXT,			5, 2,  0, 0, 0, 0, 0, 0, L"Algorithm", 0},
		/*2*/{DI_RADIOBUTTON,	6, 3,  0, 0, 0, (selectedAlgo==RHASH_CRC32), DIF_GROUP, 0, L"&1. CRC32"},
		/*3*/{DI_RADIOBUTTON,	6, 4,  0, 0, 0, (selectedAlgo==RHASH_MD5), 0, 0, L"&2. MD5"},
		/*4*/{DI_RADIOBUTTON,	6, 5,  0, 0, 0, (selectedAlgo==RHASH_SHA1), 0, 0, L"&3. SHA1"},
		/*5*/{DI_RADIOBUTTON,	6, 6,  0, 0, 0, (selectedAlgo==RHASH_SHA256), 0, 0, L"&4. SHA256"},
		/*6*/{DI_RADIOBUTTON,	6, 7,  0, 0, 0, (selectedAlgo==RHASH_SHA512), 0, 0, L"&5. SHA512"},
		/*7*/{DI_RADIOBUTTON,	6, 8,  0, 0, 0, (selectedAlgo==RHASH_WHIRLPOOL), 0, 0, L"&6. Whirlpool"},
		
		/*8*/{DI_TEXT,			3, 9,  0, 0, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR, 0, L""},
		/*9*/{DI_TEXT,			5,10,  0, 0, 0, 0, 0, 0, L"Output to", 0},
		/*10*/{DI_RADIOBUTTON,	6,11,  0, 0, 0, 1, DIF_GROUP, 0, L"&File"},
		/*11*/{DI_RADIOBUTTON,	6,12,  0, 0, 0, 0, 0, 0, L"&Separate hash files"},
		/*12*/{DI_RADIOBUTTON,	6,13,  0, 0, 0, 0, 0, 0, L"&Display"},
		/*13*/{DI_EDIT,			15,11,38, 0, 1, 0, DIF_EDITEXPAND|DIF_EDITPATH,0, L"hashlist", 0},
		
		/*14*/{DI_TEXT,			3,14,  0, 0, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR, 0, L""},
		/*15*/{DI_CHECKBOX,		5,15,  0, 0, 0, recursive, 0, 0, L"Process directories &recursively"},
		
		/*16*/{DI_TEXT,			3,16,  0, 0, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR, 0, L"", 0},
		/*17*/{DI_BUTTON,		0,17,  0,13, 0, 0, DIF_CENTERGROUP, 1, L"Run", 0},
		/*18*/{DI_BUTTON,		0,17,  0,13, 0, 0, DIF_CENTERGROUP, 0, L"Cancel", 0},
	};
	size_t numDialogItems = sizeof(DialogItems) / sizeof(DialogItems[0]);

	HANDLE hDlg = FarSInfo.DialogInit(FarSInfo.ModuleNumber, -1, -1, 45, 20, L"GenerateParams", DialogItems, numDialogItems, 0, 0, FarSInfo.DefDlgProc, 0);

	bool retVal = false;
	if (hDlg != INVALID_HANDLE_VALUE)
	{
		int ExitCode = FarSInfo.DialogRun(hDlg);
		if (ExitCode == numDialogItems - 2) // OK was pressed
		{
			recursive = DlgHlp_GetSelectionState(hDlg, 15) != 0;
			DlgHlp_GetEditBoxText(hDlg, 13, outputFileName);

			if (DlgHlp_GetSelectionState(hDlg, 2)) selectedAlgo = RHASH_CRC32;
			else if (DlgHlp_GetSelectionState(hDlg, 3)) selectedAlgo = RHASH_MD5;
			else if (DlgHlp_GetSelectionState(hDlg, 4)) selectedAlgo = RHASH_SHA1;
			else if (DlgHlp_GetSelectionState(hDlg, 5)) selectedAlgo = RHASH_SHA256;
			else if (DlgHlp_GetSelectionState(hDlg, 6)) selectedAlgo = RHASH_SHA512;
			else if (DlgHlp_GetSelectionState(hDlg, 7)) selectedAlgo = RHASH_WHIRLPOOL;

			if (DlgHlp_GetSelectionState(hDlg, 10)) outputTarget = OT_SINGLEFILE;
			else if (DlgHlp_GetSelectionState(hDlg, 11)) outputTarget = OT_SEPARATEFILES;
			else if (DlgHlp_GetSelectionState(hDlg, 12)) outputTarget = OT_DISPLAY;
			
			retVal = true;
		}
		FarSInfo.DialogFree(hDlg);
	}
	return retVal;
}

static bool CALLBACK FileHashingProgress(HANDLE context, int64_t bytesProcessed)
{
	if (CheckEsc())
		return false;

	if (context == NULL) return true;

	ProgressContext* prCtx = (ProgressContext*) context;
	prCtx->ProcessedBytes += bytesProcessed;

	int nFileProgress = (prCtx->TotalFileSize > 0) ? (int) ((prCtx->ProcessedBytes * 100) / prCtx->TotalFileSize) : 0;

	if (nFileProgress != prCtx->FileProgress)
	{
		prCtx->FileProgress = nFileProgress;

		static wchar_t szFileProgressLine[100] = {0};
		swprintf_s(szFileProgressLine, ARRAY_SIZE(szFileProgressLine), L"File: %d/%d. Progress: %2d%%", prCtx->FileIndex + 1, prCtx->TotalFilesCount, nFileProgress);

		static const wchar_t* InfoLines[4];
		InfoLines[0] = L"Processing";
		InfoLines[1] = L"Generating hash";
		InfoLines[2] = szFileProgressLine;
		InfoLines[3] = prCtx->FileName.c_str();

		FarSInfo.Message(FarSInfo.ModuleNumber, 0, NULL, InfoLines, ARRAY_SIZE(InfoLines), 0);

		// Win7 only feature
		if (prCtx->TotalFileSize > 0)
		{
			PROGRESSVALUE pv;
			pv.Completed = prCtx->ProcessedBytes;
			pv.Total = prCtx->TotalFileSize;
			FarSInfo.AdvControl(FarSInfo.ModuleNumber, ACTL_SETPROGRESSVALUE, &pv);
		}
	}
	
	return true;
}

static void RunGenerateHashes()
{
	// Generation params
	rhash_ids genAlgo = (rhash_ids) optDefaultAlgo;
	bool recursive = true;
	HashOutputTargets outputTarget = OT_SINGLEFILE;
	wstring outputFile;

	if (!AskForHashGenerationParams(genAlgo, recursive, outputTarget, outputFile))
		return;

	//TODO: check for existing hash file and ask for overwrite

	StringList filesToProcess;
	HashList hashes(genAlgo);
	wstring strPanelDir;

	// Check panel for compatibility
	PanelInfo pi = {0};
	if (!FarSInfo.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)&pi)
		|| (pi.SelectedItemsNumber <= 0) || (pi.PanelType != PTYPE_FILEPANEL))
	{
		DisplayMessage(L"Error", L"Can not work with this panel", NULL, true, true);
		return;
	}

	// Win7 only feature
	FarSInfo.AdvControl(FarSInfo.ModuleNumber, ACTL_SETPROGRESSSTATE, (void*) PS_INDETERMINATE);

	// Prepare files list
	{
		FarScreenSave screen;
		DisplayMessage(L"Processing", L"Preparing file list", NULL, false, false);

		{
			wchar_t szNameBuffer[PATH_BUFFER_SIZE];
			FarSInfo.Control(PANEL_ACTIVE, FCTL_GETPANELDIR, ARRAY_SIZE(szNameBuffer), (LONG_PTR) szNameBuffer);
			IncludeTrailingPathDelim(szNameBuffer, ARRAY_SIZE(szNameBuffer));
			strPanelDir = szNameBuffer;
		}

		for (int i = 0; i < pi.SelectedItemsNumber; i++)
		{
			size_t requiredBytes = FarSInfo.Control(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, i, NULL);
			PluginPanelItem *PPI = (PluginPanelItem*)malloc(requiredBytes);
			if (PPI)
			{
				FarSInfo.Control(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, i, (LONG_PTR)PPI);
				if ((PPI->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					filesToProcess.push_back(PPI->FindData.lpwszFileName);
				}
				else
				{
					wstring strSelectedDir = strPanelDir + PPI->FindData.lpwszFileName;
					PrepareFilesList(strSelectedDir.c_str(), PPI->FindData.lpwszFileName, filesToProcess, recursive);
				}
				free(PPI);
			}
		}
	}

	// Perform hashing
	char hashValueBuf[150] = {0};
	ProgressContext progressCtx;
	progressCtx.TotalFilesCount = filesToProcess.size();
	progressCtx.FileIndex = -1;

	bool continueSave = true;
	for (StringList::const_iterator cit = filesToProcess.begin(); cit != filesToProcess.end(); cit++)
	{
		wstring strNextFile = *cit;
		wstring strFullPath = strPanelDir + strNextFile;

		progressCtx.FileName = strNextFile;
		progressCtx.FileIndex++;
		progressCtx.ProcessedBytes = 0;
		progressCtx.TotalFileSize = GetFileSize_i64(strFullPath.c_str());
		progressCtx.FileProgress = 0;

		{
			FarScreenSave screen;
			int genRetVal = GenerateHash(strFullPath.c_str(), genAlgo, hashValueBuf, FileHashingProgress, &progressCtx);

			if (genRetVal == GENERATE_ABORTED)
			{
				// Exit silently
				continueSave = false;
				break;
			}
			else if (genRetVal == GENERATE_ERROR)
			{
				//TODO: offer retry
				DisplayMessage(L"Error", L"Error during hash generation", strNextFile.c_str(), true, true);
				continueSave = false;
				break;
			}
		}

		hashes.SetFileHash(strNextFile.c_str(), hashValueBuf);
	}

	FarSInfo.AdvControl(FarSInfo.ModuleNumber, ACTL_SETPROGRESSSTATE, (void*) PS_NOPROGRESS);
	FarSInfo.AdvControl(FarSInfo.ModuleNumber, ACTL_PROGRESSNOTIFY, 0);

	if (!continueSave) return;

	// Display/save hash list
	bool saveSuccess = false;
	if (outputTarget == OT_SINGLEFILE)
	{
		saveSuccess = hashes.SaveList(outputFile.c_str());
	}
	else if (outputTarget == OT_SEPARATEFILES)
	{
		saveSuccess = hashes.SaveListSeparate(strPanelDir.c_str());
	}
	else
	{
		saveSuccess = true;
		DisplayMessage(L"Hashing complete", L"Stub", NULL, false, true);
	}

	// Clear selection if requested
	if (saveSuccess && optClearSelectionOnComplete)
	{
		for (int i = pi.SelectedItemsNumber - 1; i >=0; i--)
			FarSInfo.Control(PANEL_ACTIVE, FCTL_CLEARSELECTION, i, NULL);
	}

	FarSInfo.Control(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, NULL);
}

static void RunComparePanels()
{
	//TODO: implement
	DisplayMessage(L"Not implemented", L"Panels Compare", NULL, false, true);
}

// ------------------------------------- Exported functions --------------------------------------------------

int WINAPI GetMinFarVersionW(void)
{
	return FARMANAGERVERSION;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	FarSInfo = *Info;
	FSF = *Info->FSF;
	FarSInfo.FSF = &FSF;

	LoadSettings();
}

void WINAPI ExitFARW( void )
{
	//
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize = sizeof(PluginInfo);
	Info->Flags = 0;

	static wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0] = L"Integrity Checker";
	static wchar_t *PluginConfigStrings[1];
	PluginConfigStrings[0] = L"Integrity Checker";

	Info->PluginMenuStrings = PluginMenuStrings;
	Info->PluginMenuStringsNumber = sizeof(PluginMenuStrings) / sizeof(PluginMenuStrings[0]);
	Info->PluginConfigStrings = PluginConfigStrings;
	Info->PluginConfigStringsNumber = sizeof(PluginConfigStrings) / sizeof(PluginConfigStrings[0]);
	Info->CommandPrefix = optPrefix;
}

int WINAPI ConfigureW(int ItemNumber)
{
	//TODO: implement
	DisplayMessage(L"Configure", L"Not implemented", NULL, true, true);
	return FALSE;
}

HANDLE WINAPI OpenPluginW(int OpenFrom, INT_PTR Item)
{
	if (OpenFrom == OPEN_COMMANDLINE)
	{
		// We are from prefix
		if (!RunValidateFiles((wchar_t*) Item, true))
			DisplayMessage(L"Error", L"File is not a valid hash list", NULL, true, true);
	}
	else if (OpenFrom == OPEN_PLUGINSMENU)
	{
		// We are from regular plug-ins menu

		FarMenuItem MenuItems[] = {
			{L"&Generate Hashes", 1, 0, 0},
			{L"Compare &Panels", 0, 0, 0}
		};

		int nMItem = FarSInfo.Menu(FarSInfo.ModuleNumber, -1, -1, 0, 0, L"Integrity Checker", NULL, NULL, NULL, NULL, MenuItems, ARRAY_SIZE(MenuItems));

		switch (nMItem)
		{
			case 0:
				RunGenerateHashes();
				break;
			case 1:
				RunComparePanels();
				break;
		}
	} // OpenFrom check
		
	return INVALID_HANDLE_VALUE;
}

HANDLE WINAPI OpenFilePluginW(const wchar_t *Name, const unsigned char *Data, int DataSize, int OpMode)
{
	RunValidateFiles(Name, true);
	return INVALID_HANDLE_VALUE;
}
