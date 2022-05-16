#include "client.h"
#include "ui_client.h"

#include <QtNetwork>
#include <QFileDialog>
#include <QCompleter>

Client::Client(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);

    // Автозаполнение адреса и порта и запрос по умолчанию
    QStringList servWordList, portWordList;
    servWordList <<tr("127.0.0.1");
    portWordList <<tr("1234");
    QCompleter* completerServ = new QCompleter(servWordList, this);
    QCompleter* completerPort = new QCompleter(portWordList, this);

    ui->ServLI->setCompleter(completerServ);
    ui->PortLI->setCompleter(completerPort);
    ui->ServLI->setPlaceholderText(tr("127.0.0.1"));
    ui->PortLI->setPlaceholderText(tr("1234"));

    payloadSize = 64 * 1024; // 64KB
    totalBytes = 0;
    //bytesWritten = 0;
    //bytesToWrite = 0;
    isOk = false;

    ui->SendB->setEnabled(false);

    tcpClient = new QTcpSocket(this);

    // Когда соединение с сервером успешно, отправляется сигнал connected () и isOK устанавливается в true
    connect(tcpClient, SIGNAL(connected()), this, SLOT(tcpConnected()));
    // Когда нажимается кнопка отправки (и isOK истинно), выдается сигнал buildConnected () для начала передачи данных
    connect(this, SIGNAL(buildConnected()), this, SLOT(startTransfer()));
    // При отключении отправляем disconnected (), для isOK установлено значение false
    connect(tcpClient, SIGNAL(disconnected()), this, SLOT(tcpDisconnected()));
    // Отображение ошибки
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
}

Client::~Client()
{
    delete ui;
}

void Client::openFile()
{
    fileName = QFileDialog::getOpenFileName(this);

    if (!fileName.isEmpty())
    {

        // Получаем фактическое имя файла
        currentImageName = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);
        ui->StatusLI->setText(tr("Файл %1 успешно открыт!").arg(currentImageName));

        if(isOk == true)
        {
            ui->SendB->setEnabled(true);
        }
    }
}

void Client::send()
{
    if(!isOk)
    {
        ui->StatusLI->setText(tr("Пожалуйста, сначала подключитесь к серверу"));
        return;
    }
    else
    {
        // передаем сигнал
        emit buildConnected();
        qDebug() << "emit buildConnected()" << endl;
    }
}

void Client::connectServer()
{
    // Инициализируем отправленный байт равным 0
    //bytesWritten = 0;
    ui->StatusLI->setText(tr("Подключение к серверу"));

    //Подключение к серверу
    tcpClient->connectToHost(ui->ServLI->text(),
                             ui->PortLI->text().toInt());

    isOk = true;
    qDebug() << "connectServer: isOk is ok" << endl;
}


void Client::startTransfer()
{
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_6);

   // Получаем данные изображения
    QImage image(fileName);
    QString imageData = getImageData(image);

    qDebug() << "fileName: " <<fileName << endl;
    //qDebug() << "imageData" << imageData << endl;

    // Зарезервируйте информационное пространство общего размера, информационное пространство размера изображения, а затем введите информацию об изображении
    sendOut << qint64(0) << qint64(0) << imageData;

    // Общий размер здесь - это сумма информации об общем размере, информации о размере изображения и фактической информации об изображении
    totalBytes += outBlock.size();
    sendOut.device()->seek(0);

    // Возвращаемся в начало outBolock, заменяем два пробела qint64 (0) информацией о фактическом размере
    sendOut << totalBytes << qint64((outBlock.size() - sizeof(qint64)*2));

    // Отправляем сигнал readyRead ()
    tcpClient->write(outBlock);

    qDebug() << "Размер изображения:" << qint64((outBlock.size() - sizeof(qint64)*2)) << endl;
    qDebug() << "Размер всего пакета:" << totalBytes << endl;
    //qDebug() << "Размер оставшихся данных после отправки структуры заголовка файла (bytesToWrite):" << bytesToWrite << endl;

    outBlock.resize(0);

    ui->StatusLI->setText(tr("Передача файла %1 успешна").arg(currentImageName));
    totalBytes = 0;
    //bytesToWrite = 0;
}

void Client::displayError(QAbstractSocket::SocketError)
{
    qDebug() << tcpClient->errorString();
    tcpClient->close();

    ui->StatusLI->setText(tr("Не подключено"));
    ui->SendB->setEnabled(true);
}

void Client::tcpConnected()
{
    isOk = true;
    ui->ConB->setText(tr("Отключить"));
    ui->StatusLI->setText(tr("Подключено"));
}

void Client::tcpDisconnected()
{
    isOk = false;
    tcpClient->abort();
    ui->ConB->setText(tr("Подключение"));
    ui->StatusLI->setText(tr("Подключение отключено"));
}

QByteArray Client::getImageData(const QImage &image)
{
    QByteArray imageData;
    QBuffer buffer(&imageData);
    image.save(&buffer, "png");
    imageData = imageData.toBase64();

    return imageData;
}

// Кнопка соединения
void Client::on_ConB_clicked()
{
    if (ui->ConB->text() == tr("Подключение"))
    {
        tcpClient->abort();
        connectServer();
    }
    else
    {
        tcpClient->abort();
    }
}

// Кнопка открытия
void Client::on_OpenB_clicked()
{
    ui->StatusLI->setText(tr("Статус: Ожидание открытия файла!"));
    openFile();
}

// Кнопка отправки
void Client::on_SendB_clicked()
{
    send();
}

