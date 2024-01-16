#include <iostream>
#include <windows.h>
#include <wincrypt.h>
#include <wincred.h>
#include <credentialprovider.h>
#include <vector>
#include <string>
#include <random>


bool Encrypt(const std::wstring& plaintext, std::vector<BYTE>& encryptedData)
{
    DATA_BLOB inData, outData;
    inData.pbData = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(plaintext.c_str()));
    inData.cbData = (plaintext.size() + 1) * sizeof(wchar_t);
    if (!CryptProtectData(&inData, NULL, NULL, NULL, NULL, CRYPTPROTECT_LOCAL_MACHINE, &outData))
    {
        DWORD dwError = GetLastError();
        return false; 
    }
    encryptedData.resize(outData.cbData);
    memcpy(encryptedData.data(), outData.pbData, outData.cbData);
    LocalFree(outData.pbData);
    return true;
}
std::wstring Decrypt(const std::vector<BYTE>& encryptedData)
{
    DATA_BLOB inData, outData;
    inData.pbData = const_cast<BYTE*>(encryptedData.data());
    inData.cbData = static_cast<DWORD>(encryptedData.size());
    outData.pbData = NULL;
    outData.cbData = 0;
    if (!CryptUnprotectData(&inData, NULL, NULL, NULL, NULL, CRYPTPROTECT_LOCAL_MACHINE, &outData))
    {
        DWORD dwError = GetLastError();
        throw std::runtime_error("Failed to decrypt data.");
    }
    std::wstring plaintext(reinterpret_cast<wchar_t*>(outData.pbData), outData.cbData / sizeof(wchar_t) - 1);
    LocalFree(outData.pbData);
    return plaintext;
}

// ����ɴ�ӡ�ַ����ϣ�ASCII 32��126��
const std::string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+{}|:<>?~-=[]\\;',./`";
std::string RndStr(size_t length) {
    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);
    std::string str;
    str.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        str += CHARACTERS[distribution(generator)];
    }
    return str;
}

std::wstring appName = L"HikLink";
bool Store(std::string& target)
{
    CREDENTIAL cred = { 0 };
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = appName.data();
    cred.CredentialBlobSize = target.size();
    cred.CredentialBlob = (LPBYTE)(target.data());
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
    return CredWrite(&cred, 0) == TRUE;
}

bool Remove() {
    return CredDelete(appName.data(), CRED_TYPE_GENERIC, 0) == TRUE;
}


std::string Read()
{
    CREDENTIAL* cred;
    if (CredRead(appName.data(), CRED_TYPE_GENERIC, 0, &cred) == FALSE) {
        return "";
    }
    std::string result(reinterpret_cast<char*>(cred->CredentialBlob),cred->CredentialBlobSize);
    CredFree(cred);
    return result;
}


/// <summary>
/// ������Windows����DPAPI����һ������ַ�����Ȼ���������Ĵ���ļ����ŵ�AppDataĿ¼��
/// �õ�ʱ������DPAPI��������Ľ��ܳ��������ڴ����ݿ⡣
/// 
/// 1.  A���Լ��ܵ����ģ�B�����޷�����
/// 2.  ���ܴ�����C++д�ģ����ܺ�����ݿ��JS���뱻������ֽ��룬�޷�����
/// 
/// ����û��ĵ��Ա������ˣ��ڿ����û��ĵ����ϣ����û�����ݣ�ִ���˽��ܳ�����ô�ǿ��Եõ��û������ݿ������
/// ������ǾͲ�����
/// </summary>
/// <returns></returns>
int main()
{

    //�������ַ���Ӧ����JS�ֽ��봫�����ģ��������Ϳ�����JS�ֽ����ﴴ��һ�����Ĵ�������
    //����洢�������ģ���������Ӧ���õ��������֮�󣬲�֪�����ܷ�ʽҲ�ò���
    auto str = RndStr(16);
    std::cout << "Rnd Str:" << str << std::endl;

    ////ûʵ������
    //std::vector<BYTE> encryptedBytes;
    //Encrypt(str, encryptedBytes);

    ////�������д���ûʵ������
    //std::string encryptedStr(reinterpret_cast<char*>(encryptedBytes.data()), encryptedBytes.size());
    //std::cout << "Encrypt Str:" << encryptedStr << std::endl;

    ////ûʵ������
    //std::wstring decryptedStr = Decrypt(encryptedBytes);
    //std::wcout << L"Decrypted str: " << decryptedStr << std::endl;
    auto flag = Remove();
    std::cout << "Remove Ok" << flag << std::endl;
    flag = Store(str);
    std::cout << "Store Ok" << flag << std::endl;
    auto result = Read();
    std::cout << "Read Ok:" << result << std::endl;



    std::cout << "Hello World!\n";
}