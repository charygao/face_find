#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "utility_tool.h"
#include <QDebug>

#define UI_CONTROL_START u8"开始"
#define UI_CONTROL_STOP u8"停止"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    std::string url("e:/2.mp4");
    mp_decoder = std::make_shared<media_decoder>(url, std::bind(&MainWindow::handle_frame, this, std::placeholders::_1));
    m_timer.start(10);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(draw()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handle_frame(info_data_ptr p_data){
    try {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto p_info = std::make_shared<info_frame>();
            if(p_info->data.size() < p_data->data_max){
                p_info->data.resize(p_data->data_max);
            }
            memcpy(p_info->data.data(), p_data->p_data, p_data->data_max);
            p_info->p_img = std::make_shared<QImage>(p_info->data.data(), p_data->width, p_data->height, QImage::Format_RGBA8888);
            m_frames.push_back(p_info);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("发生错误:"<<e.what());
    }
}
void MainWindow::draw(){
    try {
        info_frame_ptr p_info;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if(!m_frames.empty()){
                p_info = *m_frames.begin();
                m_frames.pop_front();
            }
        }
        if(p_info){
            ui->video->setPixmap(QPixmap::fromImage(*(p_info->p_img)));
        }
    } catch (const std::exception& e) {
        LOG_ERROR("发生错误:"<<e.what());
    }
}

void MainWindow::on_pb_control_clicked()
{
    auto tag = ui->pb_control->text().toStdString();
    if(UI_CONTROL_START == tag){
        mp_decoder->start();
        ui->pb_control->setText(UI_CONTROL_STOP);
    }else if(UI_CONTROL_STOP == tag){
        mp_decoder->stop();
        ui->pb_control->setText(UI_CONTROL_START);
    }
}

void MainWindow::on_MainWindow_destroyed()
{
}

void MainWindow::closeEvent(QCloseEvent *event){
    auto tag = ui->pb_control->text().toStdString();
    if(UI_CONTROL_STOP == tag){
        mp_decoder->stop();
        ui->pb_control->setText(UI_CONTROL_START);
    }
    QMainWindow::closeEvent(event);
}
