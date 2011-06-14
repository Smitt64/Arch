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
    int m_pVersion;//Версия
    int m_pFileCount;//Количество файлов
    int m_pFileBlockSize;//Размер блока с файлами
}FSHEADER;

typedef struct File
{
    int m_pSize;//Оригинальный размер файла
    int m_pCompressedSize;//Размер сжатого файла
    char m_pName[512];//Имя
    char m_pExtension[10];//Расширение
    int m_pCompressLevel;//Уровень сжатие
    int m_pPosition;//Положение в архиве
    bool m_pTemp;//Временный?
}FSFILE;

typedef struct ArchHandle
{
    QString archFName;//Файл открытого архива
    QList<File> fileList;//Таблица файлов
    QFile file;//Файл
    Header *dataHeader;//Заголовок архива
}FSHANDLE;

class FileSystem
{
public:
    //Врзвращает указатель на класс
    static FileSystem *getInst();
    //Открыть архив
    bool fsOpen(QString fname, FSHANDLE *handle);
    //Повторно открыть архив
    bool fsOpen(FSHANDLE *handle);
    //Создать архив
    bool fsCreate(QString fname, FSHANDLE *handle);
    //Закрыть архив
    void fsClose(FSHANDLE *handle);

    //Архив открыт?
    bool fsIsOpen(FSHANDLE *handle);

    //Добавить файл в архив
    bool fsAddFile(QString source, QString name, FSHANDLE *handle, int compressLevel = 2);
    bool fsAddFile(QByteArray data, QString name, FSHANDLE *handle, int compressLevel = 2);

    //Добавляет временный файл в архив
    bool fsAddTempFile(QString source, QString name, FSHANDLE *handle, int compressLevel = 2);
    bool fsAddTempFile(QByteArray data, QString name, FSHANDLE *handle, int compressLevel = 2);

    void fsFlushTempFiles(FSHANDLE *handle);
    void fsDeleteTempFiles(FSHANDLE *handle);

    //Удалить файл из архива
    bool fsDelete(QString fileName, FSHANDLE *handle);

    //Распаковать файл
    bool fsExtractFile(QString fileName, QString saveTo, FSHANDLE *handle);

    //Перезаписывает файл в архиве, если файла нет то добавляет его
    bool fsRewriteFile(QByteArray data, QString name, FSHANDLE *handle, int compressLevel);

    //Проверяет, существует ли такой файл в архиве
    bool fsHasFile(QString name, FSHANDLE *handle);

    //Возвращает индекс файла в списке
    int fsGetFileIndex(QString name, FSHANDLE *handle);

    //Возвращает последовательность байт файла
    QByteArray fsGetFile(QString name, FSHANDLE *handle);

    //Переименовывает файл
    bool fsRenameFile(QString fnmae, QString newFName, FSHANDLE *handle);

    //Выделяет имя файла из пути имя файла
    static char *fsGetFName(QString fileName);
    //Возвращает расширение файла
    static QString fsGetExstension(QString file);

    //Возвращает список каталогов, находящихся в указаном каталоге
    QStringList fsGetFolders(QString parent, FSHANDLE *handle);
    QStringList fsGetListFiles(QString parent, FSHANDLE *handle);
    QStringList fsGetFileNamesList(FSHANDLE *handle);

    //
    static QString fsGetFDir(QString fname);
private:
    //Конструктор класса
    FileSystem();
    //Прочитать список файлов
    bool fsReadContents(FSHANDLE *handle);
    //Перезаписать список файлов
    bool fsWriteFileList(FSHANDLE *handle);

    //Добавление файла в архив
    bool fsWriteFile(QByteArray data, QString name, bool temp, FSHANDLE *handle, int compressLevel);

    //Указатель на класс
    static FileSystem *m_pInstance;
};

#endif // FILESYSTEM_H
