#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QAbstractSocket>

class QTcpSocket;
class QFile;

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QDialog
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();


private:
    Ui::Client *ui;

    QTcpSocket *tcpClient;
    QFile *localFile;     // файл для отправки
    qint64 totalBytes;    // Общий размер отправленных данных
    //qint64 bytesWritten; // Размер отправленных данных
    //qint64 bytesToWrite; // оставшийся размер данных
    qint64 payloadSize;   // Размер данных, отправляемых каждый раз (64k), не используется
    QString fileName;     // Сохраняем путь к файлу
    QByteArray outBlock;  // Буфер данных, в котором хранится блок данных, который будет отправляться каждый раз

    QImage image;   //образ
    QString currentImageName;   // Название картинки

    volatile bool isOk;

private slots:
    void openFile();    // открываем файл
    void send();    //Отправить
    void connectServer();   // подключаемся к серверу
    void startTransfer();   // Отправляем данные изображения
    void displayError(QAbstractSocket::SocketError);// Обработка ошибки функцией
    void tcpConnected();    // Обновляем значение isOk, отображение кнопки и метки
    void tcpDisconnected();     // Событие отключения

    // Изображение в строку base64
    QByteArray getImageData(const QImage&);

    void on_ConB_clicked();     //подключаеся или отключаемся от сервера
    void on_OpenB_clicked();    //открываем изображение
    void on_SendB_clicked();    // отправляем изображение
signals:
    void buildConnected();// После подключения к серверу сигнал отправлен
};
#endif // CLIENT_H
