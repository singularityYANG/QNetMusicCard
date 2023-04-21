#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>

#include <QFile>
#include <QMessageBox>

#include <QMediaPlayer>
#include <QFileDialog>
#include <QStringList>
#include <QMediaPlaylist>
#include <QVector>
#include <vector>
#include <QList>
#include "MusicData.h"
//#include "volumecontrol.h"
#include "volum2.h"

#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public:
    bool eventFilter(QObject* watched,QEvent* ev) override;
    void slot_player_state_changed(QMediaPlayer::State state);       //通过状态改变，统一调度播放按钮的改变

    //将获取到大图片切割成圆角矩形
    QPixmap roundPixmap(QPixmap pixmap,int width,int height,int r);

protected:

    void onReplied(QNetworkReply *reply);

   // void GetNetJson(QString music_name);
    void GetNetJson(QString listcode);
    void GetNetJson();

    void fromNetJson(QByteArray &byteArr);

    void update_playlist();

    void updata_ui(int index);

private slots:
    //void on_pushButton_play_clicked();

    void slot_player_duration_changed(qint64 duration); //文件时长变化，更新当前播放文件名显示
    //播放文件数据总大小的信号， 它可以获得文件时间长度。
    void slot_player_position_changed(qint64 postition); //当前文件播放位置变化，更新进度显示
    //播放到什么位置的信号， 参数是以毫秒来计算的。

    void on_pushButton_play_clicked();


    void on_pushButton_next_clicked();

   // void slot_playlist_index_changed(int position);

    void on_pushButton_Previous_clicked();

   void on_pushButton_volume_clicked();

   void on_verticalSlider_valueChanged(int value);

private:
    //http
    QNetworkAccessManager * ImgNetAccessManager;
    QString ImgUrl;
    //QPixmap *currentPicture;
    QByteArray ByteImg;	//存放图片二进制数据
protected:
    //
    void onRepliedImg(QNetworkReply *replyImg);
    //请求Imgurl 并返回图片数据，传入获取到的图片资源，并调用
    void GetNetImgJson(QString url);
    //将获取到的图片传入ui->label->setPixmap(img)
    void fromJsonImg(QByteArray &JsonImg);
protected:

    void mymenu();

    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    Ui::MainWindow *ui;

    QMediaPlayer *player;

    QMediaPlaylist *playlist;

    QList<QString> *musicnamelist;

    QNetworkAccessManager * NetAccessManger;

    QVector<MusicDta> *music;

    QString durationTime;

    QString positionTime;

    QVector<MusicDta> *musicdata;

    QPixmap Pixmap;

    //volumecontrol *volum;

    QMenu *myMenu;


};
#endif // MAINWINDOW_H
