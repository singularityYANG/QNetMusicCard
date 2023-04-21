#ifndef MUSICDATA_H
#define MUSICDATA_H
#include <QString>
class MusicDta{
public:
    MusicDta(){

        Music_name = "循迹";
        Music_author = "singularity";

        Music_url = "http://fsggdf";
        Music_Img_url = "http://fcvdvdvfd";

    }
    QString Music_name;
    QString Music_author;
    //QString Music_lrc;    //暂时用不着
    QString Music_url;
    QString Music_Img_url;

};


#endif // MUSICDATA_H

