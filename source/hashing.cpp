#include "stdafx.h"
#include "hashing.h"
#include "Utils.h"

HashAlgoInfo SupportedHashes[] = {
	{RHASH_CRC32, 8, false, 1, L".sfv", L"CRC32"},
	{RHASH_MD5, 32, true, 2, L".md5", L"MD5"},
	{RHASH_SHA1, 40, true, 2, L".sha1", L"SHA-1"},
	{RHASH_SHA256, 64, true, 2, L".sha256", L"SHA-256"},
	{RHASH_SHA512, 128, true, 2, L".sha512", L"SHA-512"},
	{RHASH_WHIRLPOOL, 128, true, 2, L".wrpl", L"Whirlpool"}
};

static bool CanBeHash(const char* msg, int msgSize)
{
	for (int i = 0; i < msgSize; i++)
	{
		if (!msg[i] || !isxdigit(msg[i]))
			return false;
	}
	return true;
}

static bool CanBePath(const char* msg, int msgSize)
{
	const char* IllegalPathChars = "<>:\"|?*";
	for (int i = 0; i < msgSize; i++)
	{
		if (!msg[i]) break;
		if ((msg[i] <= 31) || (strchr(IllegalPathChars, msg[i]) != NULL))
			return false;
	}
	return true;
}

static bool IsComment(char* line)
{
	char* strPtr = line;
	while (strPtr && *strPtr)
	{
		// Comments start with semicolon
		if (*strPtr == ';')
			return true;
		
		// Spaces are allowed before semicolon
		if (!isspace(*strPtr) && *strPtr != '\t')
			return false;

		strPtr++;
	}
	return true;
}

HashAlgoInfo* GetAlgoInfo(rhash_ids algoId)
{
	for (int i = 0; i < NUMBER_OF_SUPPORTED_HASHES; i++)
	{
		if (SupportedHashes[i].AlgoId == algoId)
			return &SupportedHashes[i];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

std::string HashList::GetFileHash( const wchar_t* FileName ) const
{
	int index = GetFileRecordIndex(FileName);
	return (index >= 0) ? m_HashList[index].HashStr : "";
}

void HashList::SetFileHash( const wchar_t* FileName, std::string HashVal )
{
	int index = GetFileRecordIndex(FileName);
	if (index >= 0)
	{
		m_HashList[index].HashStr = HashVal;
	}
	else
	{
		FileHashInfo info;
		info.Filename = FileName;
		info.HashStr = HashVal;

		m_HashList.push_back(info);
	}
}

bool HashList::SaveList( const wchar_t* filepath )
{
	stringstream sstr;

	sstr << "; Generated by Integrity Checker Plugin (by Ariman)" << endl << endl;
	for (vector<FileHashInfo>::const_iterator cit = m_HashList.begin(); cit != m_HashList.end(); cit++)
	{
		const FileHashInfo& hashData = *cit;
		SerializeFileHash(hashData, sstr);
		sstr << endl;
	}

	string strData = sstr.str();
	return DumpStringToFile(strData.c_str(), strData.length(), filepath);
}

bool HashList::SaveListSeparate( const wchar_t* baseDir )
{
	wstring dirName(baseDir);
	IncludeTrailingPathDelim(dirName);

	HashAlgoInfo* algoInfo = GetAlgoInfo(m_HashId);
	if (algoInfo == NULL) return false;

	for (vector<FileHashInfo>::const_iterator cit = m_HashList.begin(); cit != m_HashList.end(); cit++)
	{
		const FileHashInfo& hashData = *cit;
		wstring destFilePath = dirName + hashData.Filename + algoInfo->DefaultExt;
		
		stringstream sstr;
		sstr << "; Generated by Integrity Checker Plugin (by Ariman)" << endl << endl;
		SerializeFileHash(hashData, sstr);
		sstr << endl;

		string strData = sstr.str();
		if (!DumpStringToFile(strData.c_str(), strData.length(), destFilePath.c_str()))
			return false;
	}
	
	return true;
}

bool HashList::LoadList( const wchar_t* filepath )
{
	char readBuf[1024];
	FILE* inputFile;

	m_HashList.clear();

	// Do not accept files larger then 128kb
	int64_t fileSize = GetFileSize_i64(filepath);
	if (fileSize > 128 * 1024) return false;
	
	if (_wfopen_s(&inputFile, filepath, L"r") != 0)
		return false;

	bool fres = true;
	HashAlgoInfo* listAlgo = NULL;
	vector<FileHashInfo> parsedList;
	wchar_t wpathBuf[PATH_BUFFER_SIZE];

	while (fgets(readBuf, sizeof(readBuf), inputFile))
	{
		TrimRight(readBuf);

		// Just skipping comment lines
		if (IsComment(readBuf)) continue;
		
		if (listAlgo == NULL)
		{
			listAlgo = DetectHashAlgo(readBuf);
			if (listAlgo == NULL)
			{
				fres = false;
				break;
			}
			m_HashId = listAlgo->AlgoId;
		}

		size_t strSize = strlen(readBuf);
		char *possibleHash, *possiblePath;

		// First check if line is too short
		if ((int)strSize < listAlgo->HashStrSize + listAlgo->NumDelimSpaces + 1)
		{
			fres = false;
			break;
		}

		if (listAlgo->HashBeforePath)
		{
			possibleHash = readBuf;
			possiblePath = readBuf + (listAlgo->HashStrSize + listAlgo->NumDelimSpaces);
		}
		else
		{
			possiblePath = readBuf;
			possibleHash = readBuf + (strSize - listAlgo->HashStrSize);
		}

		int possiblePathSize = (int) (strSize - listAlgo->HashStrSize - listAlgo->NumDelimSpaces);
		if (CanBeHash(possibleHash, listAlgo->HashStrSize) && CanBePath(possiblePath, possiblePathSize))
		{
			int numChars = MultiByteToWideChar(m_Codepage, 0, possiblePath, possiblePathSize, wpathBuf, ARRAY_SIZE(wpathBuf));
			wpathBuf[numChars] = '\0';
			
			FileHashInfo fileInfo;
			fileInfo.Filename = wpathBuf;
			fileInfo.HashStr.append(possibleHash, listAlgo->HashStrSize);
			parsedList.push_back(fileInfo);
		}
	}
	fclose(inputFile);

	if (fres)
	{
		m_HashList.insert(m_HashList.end(), parsedList.begin(), parsedList.end());
	}

	return fres;
}

int HashList::GetFileRecordIndex( const wchar_t* fileName ) const
{
	for (size_t i = 0; i < m_HashList.size(); i++)
	{
		const FileHashInfo& info = m_HashList[i];
		if (wcscmp(info.Filename.c_str(), fileName) == 0)
			return (int) i;
	}

	return -1;
}

bool HashList::DumpStringToFile( const char* data, DWORD dataSize, const wchar_t* filePath )
{
	HANDLE hFile = CreateFile(filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) return false;

	DWORD numWritten;
	bool retVal = WriteFile(hFile, data, dataSize, &numWritten, NULL) && (numWritten == dataSize);
	CloseHandle(hFile);

	return retVal;
}

void HashList::SerializeFileHash( const FileHashInfo& data, stringstream& dest )
{
	char szFilenameBuf[MAX_PATH] = {0};
	
	WideCharToMultiByte(m_Codepage, 0, data.Filename.c_str(), -1, szFilenameBuf, ARRAY_SIZE(szFilenameBuf), NULL, NULL);
	if (m_HashId == RHASH_CRC32)
		dest << szFilenameBuf << " " << data.HashStr;
	else
		dest << data.HashStr << " *" << szFilenameBuf;
}

HashAlgoInfo* HashList::DetectHashAlgo( const char* testStr )
{
	int strLen = (int) strlen(testStr);
	for (int i = 0; i < NUMBER_OF_SUPPORTED_HASHES; i++)
	{
		HashAlgoInfo& hash = SupportedHashes[i];
		
		// Check if line is too short for hash + at least 1 character for name
		if (strLen < hash.HashStrSize + hash.NumDelimSpaces + 1)
			continue;

		if (SupportedHashes[i].HashBeforePath)
		{
			const char* possiblePath = testStr + hash.HashStrSize + hash.NumDelimSpaces;
			if (CanBeHash(testStr, hash.HashStrSize) && isspace(testStr[hash.HashStrSize]) && CanBePath(possiblePath, strLen - hash.HashStrSize - hash.NumDelimSpaces))
			{
				return &hash;
			}
		}
		else
		{
			const char* possibleHash = testStr + (strLen - hash.HashStrSize);
			if (CanBeHash(possibleHash, hash.HashStrSize) && isspace(*(possibleHash - 1)) && CanBePath(testStr, strLen - hash.HashStrSize - hash.NumDelimSpaces))
			{
				return &hash;
			}
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

int GenerateHash( const wchar_t* filePath, rhash_ids hashAlgo, char* result, HashingProgressFunc progressFunc, HANDLE progressContext )
{
	HANDLE hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE) return GENERATE_ERROR;

	const size_t readBufSize = 32 * 1024;
	char readBuf[32 * 1024];
	DWORD numReadBytes;

	int retVal = GENERATE_SUCCESS;
	int64_t totalBytes = GetFileSize_i64(hFile);

	rhash hashCtx = rhash_init(hashAlgo);
	while (retVal == GENERATE_SUCCESS && totalBytes > 0)
	{
		if (!ReadFile(hFile, readBuf, readBufSize, &numReadBytes, NULL) || !numReadBytes)
		{
			retVal = GENERATE_ERROR;
			break;
		}

		totalBytes -= numReadBytes;
		rhash_update(hashCtx, readBuf, numReadBytes);

		if (progressFunc != NULL)
		{
			if (!progressFunc(progressContext, numReadBytes))
				retVal = GENERATE_ABORTED;
		}
	}
	
	if (retVal == GENERATE_SUCCESS)
	{
		rhash_final(hashCtx, NULL);
		rhash_print(result, hashCtx, hashAlgo, RHPR_HEX);
	}

	rhash_free(hashCtx);
	CloseHandle(hFile);
	return retVal;
}
