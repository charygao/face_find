#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "media_decoder.h"
#include <mutex>
#include <vector>
#include <QTimer>
#include <QImage>
#include <QCloseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    class info_frame{
    public:
        std::shared_ptr<QImage> p_img;
        std::vector<unsigned char> data;
    };
    typedef std::shared_ptr<info_frame> info_frame_ptr;

private slots:
    void draw();
    void on_pb_control_clicked();

    void on_MainWindow_destroyed();

protected:
     virtual void closeEvent(QCloseEvent *event);

private:
    virtual void handle_frame(info_data_ptr p_data);

    Ui::MainWindow *ui;
    media_decoder_ptr mp_decoder;
    std::mutex m_mutex;
    QTimer m_timer;
    info_frame_ptr mp_info;
    std::list<info_frame_ptr> m_frames;
};

#endif // MAINWINDOW_H
