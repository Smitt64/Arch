#include "filesystem.h"
#include <QtDebug>
#include <qdebug.h>
#include <QDataStream>
#include <QObject>
#include <QFileInfo>
#include <qstringlist.h>

FileSystem *FileSystem::m_pInstance = NULL;

FileSystem::FileSystem()
{
}

FileSystem *FileSystem::getInst()
{
    if(!FileSystem::m_pInstance)
        FileSystem::m_pInstance = new FileSystem();

    return FileSystem::m_pInstance;
}

bool FileSystem::fsOpen(QString fname, FSHANDLE *handle)
{
    //������� ����� ��� ������
    bool hr = false;//��������� ��������

    handle->archFName = fname;

    if(handle->file.isOpen())
        handle->file.close();

    handle->file.setFileName(fname);
    //handle->file.setPermissions(QFile::WriteOwner);
    if(handle->file.open(QFile::ReadWrite))//���� ������� ������� ����
    {
        handle->dataHeader = new Header();

        handle->file.read(((char*)(handle->dataHeader)), sizeof(Header));

#ifndef FS_DEBUG
        qDebug("Version: %i", handle->dataHeader->m_pVersion);
        qDebug("Files: %i", handle->dataHeader->m_pFileCount);
        qDebug("File block size: %i", handle->dataHeader->m_pFileBlockSize);
        qDebug("Archive size: %i", (int)handle->file.size());

        qDebug("sizeof(Header): %i", sizeof(Header));
        qDebug("sizeof(File): %i", sizeof(File));
#endif
        if(handle->dataHeader->m_pVersion != 121)//��������� ������
            return false;

        if(handle->dataHeader->m_pFileCount)
            hr = this->fsReadContents(handle);
    }

    return hr;
}

bool FileSystem::fsOpen(FSHANDLE *handle)
{
    return this->fsOpen(handle->archFName, handle);
}

bool FileSystem::fsCreate(QString fname, FSHANDLE *handle)
{
    QFile f(fname);
    if(f.open(QIODevice::WriteOnly))
    {
        //�������� ������ ���������
        Header *head = new Header();
        head->m_pFileCount = 0;
        head->m_pVersion = 121;
        head->m_pFileBlockSize = 0;

        if(f.write((const char*)head, sizeof(Header)) != sizeof(Header))
        {
            f.close();
            return false;
        }
    }
    f.close();

    this->fsOpen(fname, handle);

    return true;
}

bool FileSystem::fsReadContents(FSHANDLE *handle)
{
    handle->file.seek(handle->dataHeader->m_pFileBlockSize + sizeof(Header));//����������� ��������� � ����� ����� � �������
    int size = handle->file.size() - (handle->dataHeader->m_pFileBlockSize + sizeof(Header));
    QByteArray fileTable;
    fileTable = handle->file.read(size);
    QByteArray newTable = qUncompress(fileTable);

    File *tmpFile = (File*)newTable.constData();
    for(int i = 0; i < handle->dataHeader->m_pFileCount; i++)
        handle->fileList.push_back(tmpFile[i]);//�������� � �����

    return true;
}

bool FileSystem::fsWriteFile(QByteArray data, QString name, bool temp, FSHANDLE *handle, int compressLevel)
{
    QByteArray compressed = qCompress(data, compressLevel);

    int dataSize = compressed.size();
    handle->file.seek(handle->dataHeader->m_pFileBlockSize + sizeof(Header));
    if(handle->file.write((const char*)compressed, dataSize) == -1)
        return false;

    //������������� � ������ ������
    handle->file.seek(0);
    //�������� ���������� ������
    handle->dataHeader->m_pFileCount++;
    handle->dataHeader->m_pFileBlockSize += dataSize;
    //������������ ���������
    handle->file.write((const char*)handle->dataHeader, sizeof(Header));

    //��������� ���������� � �����
    File info = File();
    info.m_pCompressedSize = compressed.size();
    info.m_pCompressLevel = compressLevel;

    strcpy(info.m_pName, name.toLocal8Bit().data());
    strcpy(info.m_pExtension, this->fsGetExstension(name));

    info.m_pSize = data.size();
    info.m_pPosition = sizeof(Header) + handle->dataHeader->m_pFileBlockSize - dataSize;
    info.m_pTemp = temp;

    handle->fileList.push_back(info);

#ifndef FS_DEBUG
    qDebug("Source file: %s", name.toLocal8Bit().data());
    qDebug("Source extension: %s", info.m_pExtension);
    qDebug("Source file size: %i", data.size());
    qDebug("Compressed size: %i", info.m_pCompressedSize);
#endif

    //�������� ������ ����� ������
    handle->file.resize(sizeof(Header) + handle->dataHeader->m_pFileBlockSize +
                        sizeof(File) * handle->dataHeader->m_pFileCount);

#ifndef FS_DEBUG
    qDebug("New archive size: %i", handle->file.size());
#endif

    fsWriteFileList(handle);

    return true;
}

bool FileSystem::fsAddFile(QString source, QString folder, FSHANDLE *handle, int compressLevel)
{
    bool hr = false;//��������� ��������

    QFile sourceFile(source);//���� ��� ����������
    if(sourceFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = sourceFile.read(sourceFile.size());
        hr = this->fsWriteFile(data, folder, false, handle, compressLevel);
    }

    return hr;
}

bool FileSystem::fsAddFile(QByteArray data, QString name,  FSHANDLE *handle, int compressLevel)
{
    return this->fsWriteFile(data, name, false, handle, compressLevel);
}

bool FileSystem::fsAddTempFile(QString source, QString name, FSHANDLE *handle, int compressLevel)
{
    bool hr = false;//��������� ��������

    QFile sourceFile(source);//���� ��� ����������
    if(sourceFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = sourceFile.read(sourceFile.size());
        hr = this->fsWriteFile(data, name, true, handle, compressLevel);
    }

    return hr;
}

bool FileSystem::fsAddTempFile(QByteArray data, QString name, FSHANDLE *handle, int compressLevel)
{
    return this->fsWriteFile(data, name, true, handle, compressLevel);
}

bool FileSystem::fsDelete(QString fileName, FSHANDLE *handle)
{
    //����� ���� � �������
    File file = File();//���������� �� ��������� �����
    bool finded = false;//����, ������ �� ����
    int index = 0;//��� ������ � �������
    for(int i = 0; i < handle->dataHeader->m_pFileCount; i++)
        if(fileName == QString::fromLocal8Bit(handle->fileList[i].m_pName))
        {
            file = handle->fileList[i];
            finded = true;
            index = i;
        }

    //���� ���� �� ������
    if(!finded)
    {
#ifndef ARC_DEBUG
        qDebug("File [%s] not found!", fileName.toLocal8Bit().data());
#endif
        return false;
    }

#ifdef ARC_DEBUG
    qDebug("File [%s] found at index %i.", fileName.toLocal8Bit().data(), index);
#endif

    int size = file.m_pCompressedSize;//������ ����� ����� ������

    /*
        �������� ����������� ����� �� ����� �������.
        ������������ �������.
    */
    for(int i = index + 1; i < handle->dataHeader->m_pFileCount; i++)
    {
        //������������� � ������ ���������� �����
        handle->file.seek(handle->fileList[i].m_pPosition);
        //��������� ����
        QByteArray data = handle->file.read(handle->fileList[i].m_pCompressedSize);

        //�������� � ������� ��������� �����
        handle->fileList[i].m_pPosition -= size;
        //����������� ��������� � ����� ���������
        handle->file.seek(handle->fileList[i].m_pPosition);
        //�������� ����
        handle->file.write(data);
    }
    //������� ���������� � ����� �� �������
    handle->fileList.removeAt(index);
    //�������� � ��������� ������ ����� � �������
    handle->dataHeader->m_pFileBlockSize -= size;
    handle->dataHeader->m_pFileCount--;

    //������������ ���������
    handle->file.seek(0);
    handle->file.write((const char*)handle->dataHeader, sizeof(Header));

    //�������� ������ �����
    handle->file.resize(sizeof(Header) + handle->dataHeader->m_pFileBlockSize +
                        sizeof(File) * handle->dataHeader->m_pFileCount);
    //������������ ������ ������
    fsWriteFileList(handle);

    return true;
}

bool FileSystem::fsExtractFile(QString fileName, QString saveTo, FSHANDLE *handle)
{
    //����� ���� � �������
    File file;//���������� �� ��������� �����
    bool finded = false;//����, ������ �� ����
    for(int i = 0; i < handle->dataHeader->m_pFileCount; i++)
        if(fileName == QString::fromLocal8Bit(handle->fileList[i].m_pName))
        {
            file = handle->fileList[i];
            finded = true;
        }
    if(!finded)
        return false;

    //���������� �����
    QByteArray data = this->fsGetFile(fileName, handle);
    QFile f(saveTo);
    if(!f.open(QFile::WriteOnly))
        return false;

    f.write(data);
    f.close();
    return true;
}

char *FileSystem::fsGetFName(QString fileName)
{
    if(fileName.isEmpty())
        return ((char*)(""));

    QStringList list = fileName.split('\\');

    return list.last().toLatin1().data();
}

QByteArray FileSystem::fsGetFile(QString name, FSHANDLE *handle)
{
    QByteArray data;

    for(int i = 0; i < handle->dataHeader->m_pFileCount; i++)
    {
        if(name == QString::fromLocal8Bit(handle->fileList[i].m_pName))
        {
            int pos = handle->fileList[i].m_pPosition;
            int csize = handle->fileList[i].m_pCompressedSize;
            handle->file.seek(pos);
            data = handle->file.read(csize);
            break;
        }
    }

    return qUncompress(data);
}

QString FileSystem::fsGetExstension(QString file)
{
    QFileInfo inf(file);

    return inf.extension();
}

void FileSystem::fsClose(FSHANDLE *handle)
{
    handle->fileList.clear();
    if(handle->dataHeader != NULL)
    {
        delete handle->dataHeader;
        handle->dataHeader = NULL;
    }
    handle->file.close();
}

bool FileSystem::fsHasFile(QString name, FSHANDLE *handle)
{
    for(int i = 0; i < handle->dataHeader->m_pFileCount; i++)
        if(QString::fromLocal8Bit(handle->fileList[i].m_pName) == name)
            return true;
    return false;
}

int FileSystem::fsGetFileIndex(QString name, FSHANDLE *handle)
{
    for(int i = 0; i < handle->dataHeader->m_pFileCount; i++)
        if(QString::fromLocal8Bit(handle->fileList[i].m_pName) == name)
            return i;
    return -1;
}

bool FileSystem::fsRewriteFile(QByteArray data, QString name, FSHANDLE *handle, int compressLevel)
{
    //����� ���� � �������

    int index = this->fsGetFileIndex(name, handle);//��� ������ � �������

    //���� ���� �� ������
    if(index == -1)
    {
#ifdef FS_DEBUG
        qDebug("File [%s] not found!", name.toLocal8Bit().data());
#endif
        return false;
    }

#ifdef FS_DEBUG
    qDebug("File [%s] found at index %i.", name.toLocal8Bit().data(), index);
#endif

    int size = handle->fileList[index].m_pCompressedSize;//������ ����� ����� ������

    /*
        �������� ����������� ����� �� ����� �������.
        ������������ �������.
    */
    for(int i = index + 1; i < handle->dataHeader->m_pFileCount; i++)
    {
        //������������� � ������ ���������� �����
        handle->file.seek(handle->fileList[i].m_pPosition);
        //��������� ����
        QByteArray data = handle->file.read(handle->fileList[i].m_pCompressedSize);

        //�������� � ������� ��������� �����
        handle->fileList[i].m_pPosition -= size;
        //����������� ��������� � ����� ���������
        handle->file.seek(handle->fileList[i].m_pPosition);
        //�������� ����
        handle->file.write(data);
    }
    //�������� � ��������� ������ ����� � �������
    handle->dataHeader->m_pFileBlockSize -= size;

    QByteArray compressed = qCompress(data, compressLevel);

    //�������� ����� ���� � ����� ������
    handle->file.seek(sizeof(Header) + handle->dataHeader->m_pFileBlockSize);
    handle->file.write(compressed);

    //�������� ���������� � ����� �����
    handle->fileList[index].m_pCompressLevel = compressLevel;
    handle->fileList[index].m_pCompressedSize = compressed.size();
    handle->fileList[index].m_pPosition = sizeof(Header) + handle->dataHeader->m_pFileBlockSize;
    handle->fileList[index].m_pSize = data.size();

    //�������� � ��������� ������ ����� � �������
    handle->dataHeader->m_pFileBlockSize += compressed.size();

    handle->file.seek(0);
    handle->file.write((const char*)handle->dataHeader, sizeof(Header));

    //�������� ������ �����
    handle->file.resize(sizeof(Header) + handle->dataHeader->m_pFileBlockSize +
                        sizeof(File) * handle->dataHeader->m_pFileCount);
    //������������ ������ ������
    /*handle->file.seek(sizeof(Header) + handle->dataHeader->m_pFileBlockSize);
    //������������ ������ ������
    for(int i = 0; i < handle->fileList.size(); i++)
    {
        File f = handle->fileList[i];

        handle->file.write((const char*)&f, sizeof(File));
        handle->file.flush();
    }*/
    fsWriteFileList(handle);

    return true;
}

bool FileSystem::fsRenameFile(QString fname, QString newFName, FSHANDLE *handle)
{
    //��������� ������� �����
    if(!this->fsHasFile(fname, handle))
        return false;

    //��������� ����� � �������
    return true;
}

bool FileSystem::fsWriteFileList(FSHANDLE *handle)
{
    QByteArray fileTable;
    QDataStream stream(&fileTable, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_6);

    for(int i = 0; i < handle->fileList.size(); i++)
    {
        File f = handle->fileList[i];
        stream.device()->write((const char*)&f, sizeof(File));
    }

    fileTable = qCompress(fileTable, 1);
    handle->file.seek(sizeof(Header) + handle->dataHeader->m_pFileBlockSize);
    handle->file.write(fileTable);
    handle->file.flush();

    return true;
}

QString FileSystem::fsGetFDir(QString fileName)
{
    if(fileName.isEmpty())
        return "";

    int size = 0;
    for(int i = fileName.length() - 1; i > 0; i--)
    {
        if(fileName[i].toLatin1() == '/' || fileName[i].toLatin1() == '\\')
            break;
        size++;
    }
    QString name;
    for(int i = 0; i < fileName.length() - size; i++)
        name += fileName[i];

    return name;
}

void FileSystem::fsFlushTempFiles(FSHANDLE *handle)
{
    for(int i = 0; i < handle->fileList.count(); i++)
        if(handle->fileList[i].m_pTemp)
            handle->fileList[i].m_pTemp = false;

    this->fsWriteFileList(handle);
}

void FileSystem::fsDeleteTempFiles(FSHANDLE *handle)
{
    QStringList toDelete;
    for(int i = 0; i < handle->fileList.count(); i++)
        if(handle->fileList[i].m_pTemp)
            toDelete.push_back(handle->fileList[i].m_pName);

    for(int i = 0; i < toDelete.count(); i++)
        this->fsDelete(toDelete[i], handle);
}

QStringList FileSystem::fsGetFolders(QString parent, FSHANDLE *handle)
{
    QStringList list;

    for(int i = 0; i < handle->fileList.count(); i++)
    {
        QString str = QString(handle->fileList.at(i).m_pName);

        if(str.contains(parent))
        {
            str.remove(0, parent.length());

            QStringList buf = str.split('\\');
            if(buf.count() > 1 && !list.contains(buf[0]))
                list.push_back(buf[0]);
        }
    }
    list.sort();

    return list;
}

QStringList FileSystem::fsGetListFiles(QString parent, FSHANDLE *handle)
{
    QStringList list;

    for(int i = 0; i < handle->fileList.count(); i++)
    {
        QString str = QString(handle->fileList.at(i).m_pName);

        if(str.contains(parent))
        {
            str.remove(0, parent.length());

            QStringList buf = str.split('\\');
            if(buf.count() == 1)
            {
                list.push_back(buf[0]);
            }
        }
    }

    list.sort();

    return list;
}

QStringList FileSystem::fsGetFileNamesList(FSHANDLE *handle)
{
    QStringList list;

    for(int i = 0; i < handle->fileList.count(); i++)
    {
        list.push_back(QString(handle->fileList[i].m_pName));
    }

    return list;
}
