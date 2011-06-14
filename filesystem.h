#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QString>
#include <QFile>
#include <QList>
#include <QByteArray>
#include <QString>

typedef struct File FSFILE, *LPFSFILE;

typedef struct Header
{
    int m_pVersion;//������
    int m_pFileCount;//���������� ������
    int m_pFileBlockSize;//������ ����� � �������
}FSHEADER;

typedef struct File
{
    int m_pSize;//������������ ������ �����
    int m_pCompressedSize;//������ ������� �����
    char m_pName[512];//���
    char m_pExtension[10];//����������
    int m_pCompressLevel;//������� ������
    int m_pPosition;//��������� � ������
    bool m_pTemp;//���������?
}FSFILE;

typedef struct ArchHandle
{
    QString archFName;//���� ��������� ������
    QList<File> fileList;//������� ������
    QFile file;//����
    Header *dataHeader;//��������� ������
}FSHANDLE;

class FileSystem
{
public:
    //���������� ��������� �� �����
    static FileSystem *getInst();
    //������� �����
    bool fsOpen(QString fname, FSHANDLE *handle);
    //�������� ������� �����
    bool fsOpen(FSHANDLE *handle);
    //������� �����
    bool fsCreate(QString fname, FSHANDLE *handle);
    //������� �����
    void fsClose(FSHANDLE *handle);

    //����� ������?
    bool fsIsOpen(FSHANDLE *handle);

    //�������� ���� � �����
    bool fsAddFile(QString source, QString name, FSHANDLE *handle, int compressLevel = 2);
    bool fsAddFile(QByteArray data, QString name, FSHANDLE *handle, int compressLevel = 2);

    //��������� ��������� ���� � �����
    bool fsAddTempFile(QString source, QString name, FSHANDLE *handle, int compressLevel = 2);
    bool fsAddTempFile(QByteArray data, QString name, FSHANDLE *handle, int compressLevel = 2);

    void fsFlushTempFiles(FSHANDLE *handle);
    void fsDeleteTempFiles(FSHANDLE *handle);

    //������� ���� �� ������
    bool fsDelete(QString fileName, FSHANDLE *handle);

    //����������� ����
    bool fsExtractFile(QString fileName, QString saveTo, FSHANDLE *handle);

    //�������������� ���� � ������, ���� ����� ��� �� ��������� ���
    bool fsRewriteFile(QByteArray data, QString name, FSHANDLE *handle, int compressLevel);

    //���������, ���������� �� ����� ���� � ������
    bool fsHasFile(QString name, FSHANDLE *handle);

    //���������� ������ ����� � ������
    int fsGetFileIndex(QString name, FSHANDLE *handle);

    //���������� ������������������ ���� �����
    QByteArray fsGetFile(QString name, FSHANDLE *handle);

    //��������������� ����
    bool fsRenameFile(QString fnmae, QString newFName, FSHANDLE *handle);

    //�������� ��� ����� �� ���� ��� �����
    static char *fsGetFName(QString fileName);
    //���������� ���������� �����
    static QString fsGetExstension(QString file);

    //���������� ������ ���������, ����������� � �������� ��������
    QStringList fsGetFolders(QString parent, FSHANDLE *handle);
    QStringList fsGetListFiles(QString parent, FSHANDLE *handle);
    QStringList fsGetFileNamesList(FSHANDLE *handle);

    //
    static QString fsGetFDir(QString fname);
private:
    //����������� ������
    FileSystem();
    //��������� ������ ������
    bool fsReadContents(FSHANDLE *handle);
    //������������ ������ ������
    bool fsWriteFileList(FSHANDLE *handle);

    //���������� ����� � �����
    bool fsWriteFile(QByteArray data, QString name, bool temp, FSHANDLE *handle, int compressLevel);

    //��������� �� �����
    static FileSystem *m_pInstance;
};

#endif // FILESYSTEM_H
