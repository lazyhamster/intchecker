#ifndef trust_h__
#define trust_h__

struct CertificateInfo
{
	std::wstring IssuerName;
	std::wstring SubjectName;
	std::wstring SignatureAlgorithm;
};

struct DigitalSignatureInfo
{
	std::wstring ProgramName;
	std::wstring PublisherLink;
	std::wstring MoreInfoLink;
	
	std::wstring HashAlgorithm;
	std::wstring HashEncryptionAlgorithm;
	
	CertificateInfo CertInfo;
};

struct SignedFileInformation
{
	std::vector<DigitalSignatureInfo> Signatures;
};

bool FileCanHaveSignature(const wchar_t* path);
long VerifyPeSignature(const wchar_t* path, SignedFileInformation &fileInfo);

#endif // trust_h__
