#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QPainter>
#include <QSettings>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //去掉标题栏
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    //设置透明窗口 120号属性
    this->setAttribute(Qt::WA_TranslucentBackground);
    //设置窗口顶层
    this->setWindowFlags(this->windowFlags() |Qt::Tool);
    this->setStatusBar(nullptr);     //关闭底部状态栏

    this->installEventFilter(this);

    ui->label_music_name->adjustSize(); //adjushSize();   //自适应大小
    ui->label_music_name->setWordWrap(true); //自动换行

    ui->verticalSlider->hide();


    player = new QMediaPlayer;    //为player申请空间
    playlist = new QMediaPlaylist;    //播放列表

    music = new QVector<MusicDta>;
    //music = new QList<MusicDta>;

    //volum = new volumecontrol;



    musicdata = new QVector<MusicDta>;

    NetAccessManger = new QNetworkAccessManager(this);

    ImgNetAccessManager = new QNetworkAccessManager(this);

    //绑定槽，当网络请求成功后，调用onReplied();
   connect(NetAccessManger,&QNetworkAccessManager::finished,this,&MainWindow::onReplied);

   connect(ImgNetAccessManager,&QNetworkAccessManager::finished,
              this,&MainWindow::onRepliedImg);

    //QUrl url("https://api.injahow.cn/meting/?server=netease&type=url&id=2037945324");
    //player->setVolume(50);
    //player->play();
   // player->setMedia(QUrl("https://api2.52jan.com/wyy/1394167216"));
   // player->setMedia(QUrl(url));

    player->setVolume(50);      //初始音量
    player->setPlaylist(playlist);
    //player->play();     //


    //GetNetJson("循迹");

//    connect(playlist,&QMediaPlaylist::currentIndexChanged,
          //  this,&MainWindow::slot_playlist_index_changed);

    connect(player,&QMediaPlayer::stateChanged,
            this,&MainWindow::slot_player_state_changed);

    connect(player,&QMediaPlayer::positionChanged,
            this,&MainWindow::slot_player_position_changed);

    connect(player,&QMediaPlayer::durationChanged,
             this,&MainWindow::slot_player_duration_changed);

    connect(playlist,&QMediaPlaylist::currentIndexChanged,
            this,&MainWindow::updata_ui);



    //优化拖拽音乐卡顿，但拖拽没有限制当拖拽到末尾后直接到下一首，与之前valueChanged相反;
    //当在暂定状态拖拽时就无法拖拽到下一首，只能拖拽到末
    connect(ui->horizontalSlider_Progress,&QSlider::sliderMoved,
                  player,&QMediaPlayer::setPosition);

    GetNetJson();
    playlist->setPlaybackMode(QMediaPlaylist::Loop);

    mymenu();

}

MainWindow::~MainWindow()
{
    delete ui;
}

//实现鼠标拖拽
bool MainWindow::eventFilter(QObject *watched, QEvent *ev)
{

    QMouseEvent* mouseev = static_cast<QMouseEvent*>(ev);
    //判断鼠标左键按下
    static QPoint begpos;
    if(ev->type() == QEvent::MouseButtonPress){
        begpos = mouseev->globalPos() - this->pos();
    }
    //鼠标移动
    else if(ev->type() == QEvent::MouseMove &&
            mouseev->buttons() &Qt::MouseButton::LeftButton){

        this->move(mouseev->globalPos() - begpos);
       // volum->move(mouseev->globalPos() - begpos);
    }

    return false;
}


void MainWindow::slot_player_state_changed(QMediaPlayer::State state)       //通过状态改变，统一调度播放按钮的改变
{
    switch(state)
    {
    case QMediaPlayer::StoppedState:
        break;
    case QMediaPlayer::PlayingState:
        ui->pushButton_play->setStyleSheet("image: url(:/img/pic/pause.png);");
        break;
    case QMediaPlayer::PausedState:
         ui->pushButton_play->setStyleSheet("image: url(:/img/pic/play.png);");
        break;
    default:
        break;
    }
}


void MainWindow::slot_player_position_changed(qint64 postition)
{
    ui->horizontalSlider_Progress->setValue(postition);
    ui->label_time->setText(QString("-%1:%2")
                                .arg((postition/1000)/60,2,10,QChar('0'))       //分
                                .arg((postition/1000)%60,2,10,QChar('0'))        //秒
                                );

}

void MainWindow::slot_player_duration_changed(qint64 duration)
{
    ui->horizontalSlider_Progress->setMaximum(duration); //设置进度条最大值 也就是歌曲时长 ms
    //ui->horizontalSlider_Progress->setRange(0,duration);
}

void MainWindow::GetNetJson()
{
    //QUrl url("https://api.injahow.cn/meting/?type=playlist&id=3779629");

                                //2023网易云最火热门歌曲推荐--463884947
                                //美国Billboard榜  60198
                                //热歌榜       3778678
                                // 飙升榜      19723756
    QUrl url("https://api2.52jan.com/music/songlist?server=wyy&id=19723756");
    NetAccessManger->get(QNetworkRequest(url));
}

void MainWindow::GetNetJson(QString listcode)
{
    QUrl url("https://api2.52jan.com/music/songlist?server=wyy&id="+listcode);
    NetAccessManger->get(QNetworkRequest(url));
}

void MainWindow::onReplied(QNetworkReply *reply)
{
    qDebug()<<"Net successed";

    int satus_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //qDebug() 用于测试和查看网络信息
    qDebug()<<"operation: "<<reply->operation();
    qDebug()<<"satus_code: "<<satus_code;
    qDebug()<<"url: "<<reply->url();
   // qDebug()<<"raw header: "<<reply->rawHeaderList();
    //satus_code 为200 表示请求成功 常见状态码 404 403 等
    if(reply->error() != QNetworkReply::NoError||satus_code != 200 ){
        qDebug()<<reply->errorString().toLatin1().data();
        QMessageBox::warning(this,"QtNetImg",
                             "网络连接失败",QMessageBox::Ok);
    }else{
        QByteArray byteArray = reply->readAll();
       // qDebug()<<"read all:" <<byteArray.data();
        //将获取到的所有信息通过readAll()传给fromNetJson
        //用于对数据解析
        //添加成员函数
        fromNetJson(byteArray);
    }
    reply->deleteLater();

}



void MainWindow::fromNetJson(QByteArray &byteArr){
    QJsonDocument doc = QJsonDocument::fromJson(byteArr);   //转化为json文档

    if(!doc.isArray()){
        qDebug()<<"not an jsonArrary!";
        return;
    }
    QJsonArray Allarr = doc.array();
    for(int i = 0 ;i < Allarr.size(); i++){
        QJsonObject musicobj =  Allarr[i].toObject();

//        QString music_name = musicobj["name"].toString();
//        QString music_author = musicobj["artist"].toString();
//        QString music_url = musicobj["url"].toString();
//        QString music_img_url = musicobj["pic"].toString();

        QString music_name = musicobj["title"].toString();
        QString music_author = musicobj["author"].toString();
        QString music_url = "http://music.163.com/song/media/outer/url?id="+musicobj["url"].toString().split('/').at(4);
        QString music_img_url = musicobj["pic"].toString();
        //qDebug()<<"  "<<"music_name"<<":"<<music_name<<", music_author" <<":"<<music_author;

        MusicDta music;
        music.Music_name =music_name;
        music.Music_url = music_url;
        music.Music_author = music_author;
        music.Music_Img_url = music_img_url;

        musicdata->append(music);
    }

    for(int i = 0;i < musicdata->size();i++){
        qDebug()<<musicdata->at(i).Music_name;
        qDebug()<<musicdata->at(i).Music_author;
        qDebug()<<musicdata->at(i).Music_url;
        qDebug()<<musicdata->at(i).Music_Img_url;
        qDebug()<<"---------------";
    }
    update_playlist();
}



void MainWindow::update_playlist()
{
    for(int i = 0;i<musicdata->size();i++)
    {
        //playlist->addMedia(QUrl(music->at(i).Music_url));
       // playlist->addMedia(QUrl((*it).Music_url));
        //musicnamelist->push_back((*it).Music_name);
        //QString str = music->at(i).Music_name;
        //QString str = (*it).Music_name;
        //ui->label_music_name->setText(str);
        //ui->label_music_name->setText()
        playlist->addMedia(QUrl(musicdata->at(i).Music_url));

    }

    playlist->setCurrentIndex(1);
    player->setPlaylist(playlist);
}

void MainWindow::on_pushButton_play_clicked()
{

    if(QMediaPlayer::PlayingState == player->state()){
        player->pause();
       // ui->pushButton_play->setStyleSheet("image: url(:/img/pic/play.png);");
    }
    else{
        player->play();
      // ui->pushButton_play->setStyleSheet("image: url(:/img/pic/pause.png);");
    }

}

void MainWindow::on_pushButton_next_clicked()
{
    playlist->next();
    int index = playlist->currentIndex();
    qDebug()<<index;
}

void MainWindow::on_pushButton_Previous_clicked()
{
    playlist->previous();
    int index = playlist->currentIndex();
    qDebug()<<index;
}


void MainWindow::updata_ui(int index){
    ui->label_music_name->adjustSize();

    ui->label_music_name->setText(musicdata->at(index).Music_name);
    ui->label_author->setText(musicdata->at(index).Music_author);
    QString url = musicdata->at(index).Music_Img_url;
    qDebug()<<"-----------------";
    qDebug()<<url;

    GetNetImgJson(url);
}


void MainWindow::on_pushButton_volume_clicked()
{
    if(ui->verticalSlider->isHidden()){
        ui->verticalSlider->show();
    }else{
        ui->verticalSlider->hide();
    }

}

//调节音量
void MainWindow::on_verticalSlider_valueChanged(int value)
{
    player->setVolume(value);
}

void MainWindow::GetNetImgJson(QString url)
{
    ImgNetAccessManager->get(QNetworkRequest(url));
}

void MainWindow::onRepliedImg(QNetworkReply *replyImg)
{
    qDebug()<<"图片资源请求成功";
    int satus_code = replyImg->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"operation: "<<replyImg->operation();
    qDebug()<<"satus_code: "<<satus_code;
    qDebug()<<"url: "<<replyImg->url();
    //qDebug()<<"raw header: "<<replyImg->rawHeaderList();
    if(replyImg->error() != QNetworkReply::NoError ||satus_code != 200 ){
        qDebug()<<replyImg->errorString().toLatin1().data();
        QMessageBox::warning(this,"QtNetImg","网络连接失败",QMessageBox::Ok);
    }else{
          ByteImg = replyImg->readAll();
        //这里调用fromJsonImg(),将获取到的数据传入进行解析，并显示
          fromJsonImg(ByteImg);
    }
    replyImg->deleteLater();
}

QPixmap MainWindow::roundPixmap(QPixmap pixmap,int width,int height,int r)
{
    QPixmap pix(pixmap);
    pix = pix.scaled(QSize(width, height), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPixmap roundedPixmap(pix.size());
    roundedPixmap.fill(Qt::transparent);

    QPainterPath path;
    path.addRoundedRect(QRectF(pix.rect()), r, r);

    QPainter painter(&roundedPixmap);
    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setClipPath(path);
    painter.drawPixmap(pix.rect(), pix);

    return roundedPixmap;
}

void MainWindow::fromJsonImg(QByteArray &ByteImg)
{
    //定义QPixmap类 对象
    //QPixmap Pixmap;
    //loadFromData方法，
    Pixmap.loadFromData(ByteImg);
    if (Pixmap.loadFromData(ByteImg)) {
    //加载成功，使用pixmap对象

    /*QPixmap pixmap(Pixmap);
    pixmap = pixmap.scaled(QSize(400, 400), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPixmap roundedPixmap(pixmap.size());
    roundedPixmap.fill(Qt::transparent);

    QPainterPath path;
    path.addRoundedRect(QRectF(pixmap.rect()), 25, 25);

    QPainter painter(&roundedPixmap);
    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setClipPath(path);
    painter.drawPixmap(pixmap.rect(), pixmap);
*/
    ui->label_pic->setScaledContents(true);		//设置可以缩放
    ui->label_pic->setPixmap(roundPixmap(Pixmap,400,400,25));
   // ui->widget->setStyleSheet("")


    ui->widget->setAutoFillBackground(true);

    QPalette palette= ui->widget->palette();


    Pixmap = Pixmap.copy(200,200,ui->widget->width(),ui->widget->height());
    QPixmap bkimg = roundPixmap(Pixmap,ui->widget->width(),ui->widget->height(),15);
    QPainter painter(&bkimg);
    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);

    palette.setBrush(QPalette::Window,QPixmap(bkimg));

    ui->widget->setPalette(palette);

    } else {
    //加载失败
    qDebug()<<"图片资源加载失败,或者不支持当前图片格式";
    //ui->label_pic->setText("图片资源加载失败,或者不支持当前图片格式");
    }
}


void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    //右键弹出菜单
     myMenu->exec(QCursor::pos());
     event->globalPos();
}

void MainWindow::mymenu()
{

    myMenu = new QMenu("歌曲表单");

    QAction *mExitAct;  //退出行为
    myMenu = new QMenu("歌曲表单");
    QAction *mes = new QAction(tr("关于"), this);
    mes->setIcon(QIcon(":/pic/res/info.png"));
    myMenu->addAction(mes);
    connect(mes, &QAction::triggered,
            this,  [=]{
        QMessageBox::information(this,"author", "code by singularity");
        });

    QMenu *Switch_playlists = new QMenu(tr("切换歌单"),this);
    Switch_playlists->setIcon(QIcon(":/img/pic/R-C.png"));
    myMenu->addMenu(Switch_playlists);

    QAction *Soaring_list = new QAction(QIcon(":/img/pic/Soaring_list.jpg"),tr("飙升榜"),this);
    QAction *New_Song_Chart = new QAction(QIcon(":/img/pic/New_Song_Chart.jpg"),tr("新歌榜"),this);
    QAction *Original_list = new QAction(QIcon(":/img/pic/Original_list.jpg"),tr("原创榜"),this);
    QAction *Hot_Song_Chart = new QAction(QIcon(":/img/pic/Hot_Song_Chart.jpg"),tr("热歌榜"),this);
    QAction *tiktok = new QAction(QIcon(":/img/pic/douyin.ico"),tr("抖音热榜"),this);

    Switch_playlists->addAction(Soaring_list);
    Switch_playlists->addAction(New_Song_Chart);
    Switch_playlists->addAction(Original_list);
    Switch_playlists->addAction(Hot_Song_Chart);
    Switch_playlists->addAction(tiktok);

    //热歌榜       3778678
    // 飙升榜      19723756
    connect(Soaring_list,&QAction::triggered,this,[=]{
       qDebug()<<"飙升榜";
       music->clear();
       playlist->clear();
       musicdata->clear();
       GetNetJson("19723756");
    });
    connect(New_Song_Chart,&QAction::triggered,this,[=]{
       qDebug()<<"新歌榜";
       music->clear();
       playlist->clear();
       musicdata->clear();

    });
    connect(Original_list,&QAction::triggered,this,[=]{
       qDebug()<<"原创榜";//2884035
       music->clear();
       playlist->clear();
       musicdata->clear();
       GetNetJson("2884035");
    });
    connect(Hot_Song_Chart,&QAction::triggered,this,[=]{
       qDebug()<<"热歌榜"; //3778678
       music->clear();
       playlist->clear();
       musicdata->clear();
       GetNetJson("3778678");
    });

    connect(tiktok,&QAction::triggered,this,[=]{
        //2851159417
        qDebug()<<"tiktok"; //3778678
        music->clear();
        playlist->clear();
        musicdata->clear();
        GetNetJson("2947289424");
    });

    mExitAct = new QAction(QIcon(":/pic/res/logout.png"),"退出");

    myMenu->addAction(mExitAct);
    connect(mExitAct,&QAction::triggered,this,[=]{
        qApp->exit(0);
    });
}

