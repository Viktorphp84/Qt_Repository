#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QValidator>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentWidget(ui->tab);

    QString txt = "U%";
    ui->widget->xAxis->setLabel("Номер участка");
    ui->widget->yAxis->setLabel(QChar(916) + txt);
    ui->widget_2->xAxis->setLabel("Номер участка");
    ui->widget_2->yAxis->setLabel(QChar(916) + txt);

    //Установка шага в 1ед для осей X обоих графиков
    QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
    ui->widget->xAxis->setTicker(fixedTicker);
    ui->widget_2->xAxis->setTicker(fixedTicker);
    fixedTicker->setTickStep(1.0);
    fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);

    model = new ComboBoxModel();

    ui->checkBox->setChecked(false);
    ui->radioButton->setChecked(true);

    //Валидация строки ввода количества потребителей
    QRegularExpression reg("[0-9]+");
    QValidator* valid = new QRegularExpressionValidator(reg, this);
    ui->lineEdit->setValidator(valid);

    setWindowIcon(QIcon(":/icon.ico"));//установка иконки

    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::pushButton_2_clicked);
    //connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::slot_Calculation_Of_Loads);

    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::slot_Set_Validator_For_Activ_Power);

    //обнуление данных
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::slot_Delete_Line_Window_1);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::slot_Delete_Line_Window_2);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::slot_Cleaning_Graphics_Recalculation);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::slot_Delete_Line_Window_4);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//Архитектура модель-представление для полей выбора типа провода
/********************************************************************************************************************/
ComboBoxModel::ComboBoxModel(QObject* parent):QAbstractListModel(parent)
{
    values = new QList<the_Wire>();

    values->append({"3x16+1x25", {2.448, 1.770, 0.0865, 0.0739, 16}});
    values->append({"3x25+1x35", {1.540, 1.770, 0.0827, 0.0703, 25}});
    values->append({"3x35+1x50", {1.111, 1.262, 0.0802, 0.0691, 35}});
    values->append({"3x50+1x70", {0.822, 0.632, 0.0799, 0.0685, 50}});
    values->append({"3x70+1x95", {0.568, 0.527, 0.0789, 0.0669, 70}});
    values->append({"3x95+1x95", {0.411, 0.527, 0.0762, 0.0656, 95}});
    values->append({"3x120+1x95", {0.325, 0.527, 0.0745, 0.0650, 120}});
    values->append({"3x150+1x95", {0.265, 0.527, 0.0730, 0.0647, 150}});
    values->append({"3x185+1x95", {0.211, 0.527, 0.0723, 0.0649, 185}});
    values->append({"3x240+1x95", {0.162, 0.527, 0.0705, 0.0647, 240}});

    populate(values);
}

int ComboBoxModel::rowCount(const QModelIndex &) const
{
    return values->count();
}

QVariant ComboBoxModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())return QVariant();

    QVariant value;
    switch (role)
    {
        case Qt::DisplayRole:
        {
            value = this->values->value(index.row()).name;
        }
        break;
        case Qt::UserRole:
        {
            value = QVariant::fromValue(this->values->value(index.row()).data_Wire);
        }
        break;
        default:break;
    }
    return value;
}

void ComboBoxModel::populate(QList<the_Wire>* newValues)
{
    int ind = this->values->count();
    this->beginInsertRows(QModelIndex(), 1, ind);
    this->values = newValues;
    endInsertRows();
}
/********************************************************************************************************************/

//Функция для расчета средневзвешенного косинуса
/********************************************************************************************************************/
void MainWindow::calculation_Of_WeightedAverage_Coefficient()
{
    double Pcos = 0;//сумма произведений активной мощности потребителя на косинус
    double weighted_Average_Coefficient = 0;//средневзвешенный косинус
    double P = 0;//сумма активных мощностей

    for(unsigned int j = 0; j < number_of_consumers - 1; ++j)
    {
        for(unsigned int i  = j; i < number_of_consumers; ++i)
        {
            Pcos = Pcos + vector_For_Active_Power_Consumer[i]
                    * vector_For_Cofficient_Of_Active_Power[i];
            P = P + vector_For_Active_Power_Consumer[i];
        }
        weighted_Average_Coefficient = Pcos / P;
        vector_Weighted_Average_Coefficient.push_back(weighted_Average_Coefficient);
    }
    vector_Weighted_Average_Coefficient.push_back(vector_For_Cofficient_Of_Active_Power[number_of_consumers - 1]);
}
/********************************************************************************************************************/

//Слот для установки количества строк
/********************************************************************************************************************/
void MainWindow::on_lineEdit_editingFinished()
{
    number_of_consumers = ui->lineEdit->text().toInt();//сохранение количества строк
    number = number_of_consumers;//вспомогательная переменная

    vector_Link_To_Wire.clear();

    slot_Delete_Line_Window_1();
    slot_Delete_Line_Window_2();
    slot_Delete_Line_Window_4();
    slot_Cleaning_Graphics_Recalculation();
    ui->comboBox->setCurrentIndex(0);

    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        //Создание строк для ввода активных нагрузок
        /*********************************************************************************************************/
        QHBoxLayout* hBox = new QHBoxLayout();
        QString string = "";
        QLabel* lab = new QLabel("№" + string.setNum(i + 1));
        QLineEdit* line = new QLineEdit();
        line->setMaximumWidth(110);

        QRegularExpression reg_1("[0-1]{1}\\.{1,1}[0-9]+|[0-2]{1}");
        QValidator* valid_1 = new QRegularExpressionValidator(reg_1, line);
        line->setValidator(valid_1);

        hBox->addStretch();
        hBox->addWidget(lab);
        hBox->addWidget(line);
        hBox->addStretch();
        ui->verticalLayout->addLayout(hBox);
        /*********************************************************************************************************/

        //Создание строк для ввода длин участков
        /*********************************************************************************************************/
        QHBoxLayout* hBox_2 = new QHBoxLayout();
        QString string_2 = "";
        QLabel* lab_2 = new QLabel("№" + string_2.setNum(i + 1));
        QLineEdit* line_2 = new QLineEdit();
        line_2->setMaximumWidth(110);

        //Установка валидатора для строк ввода длин участков
        QRegularExpression reg_2("\\d+\\.?[0-9]+");
        QValidator* valid_2 = new QRegularExpressionValidator(reg_2, line_2);
        line_2->setValidator(valid_2);

        hBox_2->addStretch();
        hBox_2->addWidget(lab_2);
        hBox_2->addWidget(line_2);
        hBox_2->addStretch();
        ui->verticalLayout_2->addLayout(hBox_2);
        /*********************************************************************************************************/

        //Создание строк для ввода значений косинусов
        /*********************************************************************************************************/
        QHBoxLayout* hBox_3 = new QHBoxLayout();
        QString string_3 = "";
        QLabel* lab_3 = new QLabel("№" + string_3.setNum(i + 1));
        QLineEdit* line_3 = new QLineEdit();
        line_3->setMaximumWidth(110);

        //Установка валидатора для строк ввода косинусов
        QRegularExpression reg_3("[0]{1,1}[.]{1,1}[0-9]+");
        QValidator* valid_3 = new QRegularExpressionValidator(reg_3, line_3);
        line_3->setValidator(valid_3);

        hBox_3->addStretch();
        hBox_3->addWidget(lab_3);
        hBox_3->addWidget(line_3);
        hBox_3->addStretch();
        ui->verticalLayout_3->addLayout(hBox_3);
        /*********************************************************************************************************/

        //Создание строк для ввода значений сечений проводов
        /*********************************************************************************************************/
        QHBoxLayout* hBox_4 = new QHBoxLayout();
        QString string_4 = "";
        QLabel* lab_4 = new QLabel("№" + string_4.setNum(i + 1));
        QComboBox* comboBox_4 = new QComboBox();
        vector_Link_To_Wire.append(comboBox_4);
        comboBox_4->setMaximumWidth(110);


        hBox_4->addStretch();
        hBox_4->addWidget(lab_4);
        hBox_4->addWidget(comboBox_4);
        hBox_4->addStretch();
        ui->verticalLayout_4->addLayout(hBox_4);

        comboBox_4->setModel(model);
        /*********************************************************************************************************/
    }
    //Создание строк для ввода активных нагрузок
    QWidget* widg = new QWidget(ui->scrollArea);
    widg->setLayout(ui->verticalLayout);
    ui->scrollArea->setWidget(widg);
    //Создание строк для ввода длин участков
    /*************************************************************************************************************/
    QWidget* widg_2 = new QWidget(ui->scrollArea_2);
    widg_2->setLayout(ui->verticalLayout_2);
    ui->scrollArea_2->setWidget(widg_2);
    //Создание строк для ввода значений косинусов
    /*************************************************************************************************************/
    QWidget* widg_3 = new QWidget(ui->scrollArea_3);
    widg_3->setLayout(ui->verticalLayout_3);
    ui->scrollArea_3->setWidget(widg_3);
    //Создание строк для ввода значений сечений проводов
    /*************************************************************************************************************/
    QWidget* widg_4 = new QWidget(ui->scrollArea_4);
    widg_4->setLayout(ui->verticalLayout_4);
    ui->scrollArea_4->setWidget(widg_4);
}
/********************************************************************************************************************/

//Слот для обнуления строк в первом окне
/********************************************************************************************************************/
void MainWindow::slot_Delete_Line_Window_1()
{
    QObject* obj = sender();
    if(obj == ui->pushButton)
    {
        ui->lineEdit->setText("");//обнуление строки ввода количества потребителей
    }

    //обнуление векторов
    vector_Weighted_Average_Coefficient.clear();
    vector_For_Active_Power_Consumer.clear();
    vector_For_Length_Of_Site.clear();
    vector_For_Cofficient_Of_Active_Power.clear();
    vector_For_Full_Power_Of_Site.clear();
    vector_Calculation_Of_Loads.clear();
    vector_For_Delta_U.clear();
    vector_For_U_In_Percents.clear();
    vector_Link_To_Wire.clear();
    vector_Sum_For_U_In_Persents.clear();

    //Удаление строк в окне исходных данных
    /*************************************************************************************************************/
    QList <QLabel*> labList = ui->scrollArea->findChildren <QLabel*> ();
    foreach(QLabel* lab, labList)
    {
        delete lab;
    }

    QList <QLineEdit*> lineList = ui->scrollArea->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line, lineList)
    {
        delete line;
    }

    QList <QLabel*> labList_2 = ui->scrollArea_2->findChildren <QLabel*> ();
    foreach(QLabel* lab_2, labList_2)
    {
        delete lab_2;
    }

    QList <QLineEdit*> lineList_2 = ui->scrollArea_2->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_2, lineList_2)
    {
        delete line_2;
    }

    QList <QLabel*> labList_3 = ui->scrollArea_3->findChildren <QLabel*> ();
    foreach(QLabel* lab_3, labList_3)
    {
        delete lab_3;
    }

    QList <QLineEdit*> lineList_3 = ui->scrollArea_3->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_3, lineList_3)
    {
        delete line_3;
    }

    QList <QLabel*> labList_4 = ui->scrollArea_4->findChildren <QLabel*> ();
    foreach(QLabel* lab_4, labList_4)
    {
        delete lab_4;
    }

    QList <QComboBox*> comboList_4 = ui->scrollArea_4->findChildren <QComboBox*> ();
    foreach(QComboBox* combo_4, comboList_4)
    {
        delete combo_4;
    }
    /*************************************************************************************************************/ 
}
/********************************************************************************************************************/

//Слот для обнуления строк во втором окне
/********************************************************************************************************************/
void MainWindow::slot_Delete_Line_Window_2()
{
    //Удаление строк в окне вывода нагрузок по участкам
    /*************************************************************************************************************/
    QList <QLabel*> labList_5 = ui->scrollArea_5->findChildren <QLabel*> ();
    foreach(QLabel* lab_5, labList_5)
    {
        delete lab_5;
    }

    QList <QLineEdit*> lineList_5 = ui->scrollArea_5->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_5, lineList_5)
    {
        delete line_5;
    }

    QList <QLabel*> labList_6 = ui->scrollArea_6->findChildren <QLabel*> ();
    foreach(QLabel* lab_6, labList_6)
    {
        delete lab_6;
    }

    QList <QLineEdit*> lineList_6 = ui->scrollArea_6->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_6, lineList_6)
    {
        delete line_6;
    }

    QList <QLabel*> labList_7 = ui->scrollArea_7->findChildren <QLabel*> ();
    foreach(QLabel* lab_7, labList_7)
    {
        delete lab_7;
    }

    QList <QLineEdit*> lineList_7 = ui->scrollArea_7->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_7, lineList_7)
    {
        delete line_7;
    }

    QList <QLabel*> labList_8 = ui->scrollArea_8->findChildren <QLabel*> ();
    foreach(QLabel* lab_8, labList_8)
    {
        delete lab_8;
    }

    QList <QLineEdit*> lineList_8 = ui->scrollArea_8->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_8, lineList_8)
    {
        delete line_8;
    }

    QList <QLabel*> labList_13 = ui->scrollArea_13->findChildren <QLabel*> ();
    foreach(QLabel* lab_13, labList_13)
    {
        delete lab_13;
    }

    QList <QLineEdit*> lineList_13 = ui->scrollArea_13->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_13, lineList_13)
    {
        delete line_13;
    }
    /*************************************************************************************************************/
}
/********************************************************************************************************************/

//Слот для записи исходных данных в вектора
/********************************************************************************************************************/
void MainWindow::pushButton_2_clicked()
{
    vector_For_Active_Power_Consumer.clear();
    vector_For_Length_Of_Site.clear();
    vector_For_Cofficient_Of_Active_Power.clear();
    vec_Return_Index_For_ComboBox.clear();
    bool_Vec_Temp.clear();

    flag_Message = false;
    flag_For_Line = false;

    //Запись активных нагрузок
    QList <QLineEdit*> lineListClicked = ui->scrollArea->findChildren <QLineEdit*> ();
    foreach(QLineEdit* lineListClick, lineListClicked)
    {
        double x = lineListClick->text().toDouble();
        vector_For_Active_Power_Consumer.push_back(x);
        if(!x)flag_For_Line = true;
    }

    //Запись длин участков
    QList <QLineEdit*> lineListClicked_2 = ui->scrollArea_2->findChildren <QLineEdit*> ();
    foreach(QLineEdit* lineListClick_2, lineListClicked_2)
    {
        double x = lineListClick_2->text().toDouble();
        vector_For_Length_Of_Site.push_back(x);
        if(!x)flag_For_Line = true;
    }

    //Запись коэффициентов активной мощности
    QList <QLineEdit*> lineListClicked_3 = ui->scrollArea_3->findChildren <QLineEdit*> ();
    foreach(QLineEdit* lineListClick_3, lineListClicked_3)
    {
        double x = lineListClick_3->text().toDouble();
        vector_For_Cofficient_Of_Active_Power.push_back(x);
        if(!x)flag_For_Line = true;
    }

    //Запись индексов ComboBox для каждого участка
    QList <QComboBox*> lineListClicked_4 = ui->scrollArea_4->findChildren <QComboBox*> ();
    foreach(QComboBox* lineListClick_4, lineListClicked_4)
    {
        int x = lineListClick_4->currentIndex();
        vec_Return_Index_For_ComboBox.push_back(x);
    }

    vec_Pair_Index.clear();
    vec_Bool_For_Pair.clear();
    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        if(vector_For_Length_Of_Site[i] >= 0.8)
        {
            vec_Pair_Index.push_back({vector_Link_To_Wire[i]->currentIndex(), vector_Link_To_Wire[i]->currentIndex()});
        }
        else
        {
            vec_Pair_Index.push_back({-1, -1});
        }
        vec_Bool_For_Pair.push_back(false);
    }

    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        bool_Vec_Temp.push_back(true);
    }

    if(flag_For_Line)
    {
        QMessageBox message;
        message.setIcon(QMessageBox::Warning);
        message.setText("Заполните все поля с данными!");
        message.exec();

        slot_Delete_Line_Window_2();
        slot_Cleaning_Graphics_Recalculation();
        slot_Delete_Line_Window_4();
    }
    else
    {
        slot_Calculation_Of_Loads();
    }
}
/********************************************************************************************************************/

//Функция для расчета отличия нагрузок и сохранения максимальной нагузки
/********************************************************************************************************************/
bool MainWindow::difference_Consumers(unsigned int current_Position)
{
    Pmax = vector_For_Active_Power_Consumer[current_Position];
    bool b = true;

    for(unsigned int i = current_Position; i < number_of_consumers; ++i)
    {
        for(unsigned int j = i + 1; j < number_of_consumers; ++j)
        {
           if(((vector_For_Active_Power_Consumer[i] > vector_For_Active_Power_Consumer[j])
                   && ((vector_For_Active_Power_Consumer[i] / vector_For_Active_Power_Consumer[j]) > 4))
                   || ((vector_For_Active_Power_Consumer[i] < vector_For_Active_Power_Consumer[j])
                   && ((vector_For_Active_Power_Consumer[j] / vector_For_Active_Power_Consumer[i]) >= 4)))
           {
               b = false;
           }
        }
        if(Pmax < vector_For_Active_Power_Consumer[i]) Pmax = vector_For_Active_Power_Consumer[i];
    }
    return b;
}
/********************************************************************************************************************/

//Слот для установки текущего типа нагрузки
/********************************************************************************************************************/
void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    current_Type_Of_Load = index;
}
/********************************************************************************************************************/

//Шаблон функции для вычисления коэффициентов одновременности и добавок к мощностям
/********************************************************************************************************************/
template<typename T1, typename T2, typename T3>
double MainWindow::finding_CoffSimul_and_PowerSuppl(T1& map, T2& number, T3 iterator)
{
   for(iterator = map.begin(); iterator != map.end(); ++iterator)
   {
       if(number >= iterator.key()
               && number < (++iterator).key())
       {
           double P2_key = iterator.key();
           double P2_value = iterator.value();
           --iterator;
           double temp_Number = number - iterator.key();
           double P1_key = iterator.key();
           double P1_value = iterator.value();
           double delta_P_key = P2_key - P1_key;
           double delta_P_value = P2_value - P1_value;
           double x = (temp_Number * delta_P_value) / delta_P_key;
           return P1_value + x;
       }
       --iterator;
   }
}
/********************************************************************************************************************/

//Метод для расчета отклонения напряжения
/********************************************************************************************************************/
void MainWindow::calculation_Of_Voltage_Deviation()
{
    //Расчет отклонения напряжения
    /*************************************************************************************************************/
    vector_For_Delta_U.clear();
    vector_For_U_In_Percents.clear();
    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        double delta_U = 0;
        if(vector_For_Length_Of_Site[i] >= 0.8)
        {
            QModelIndex model_Index_1 = model->index(vec_Pair_Index[i].first, 0, QModelIndex());
            QVariant resist_1 = model->data(model_Index_1, Qt::UserRole);
            data_Wire res_1 = resist_1.value<data_Wire>();
            double activResistMainCore_1 = res_1.activ_Resistance_Of_The_Main_Core;
            double reactivResistMainCore_1 = res_1.reactiv_Resistance_Of_The_Main_Core;

            QModelIndex model_Index_2 = model->index(vec_Pair_Index[i].second, 0, QModelIndex());
            QVariant resist_2 = model->data(model_Index_2, Qt::UserRole);
            data_Wire res_2 = resist_2.value<data_Wire>();
            double activResistMainCore_2 = res_2.activ_Resistance_Of_The_Main_Core;
            double reactivResistMainCore_2 = res_2.reactiv_Resistance_Of_The_Main_Core;

            delta_U = (vector_For_Full_Power_Of_Site[i] * ((activResistMainCore_1 + activResistMainCore_2)
                                                                  * vector_Weighted_Average_Coefficient[i]
                             + (reactivResistMainCore_1 + reactivResistMainCore_2)
                                                                  * sqrt(1 - vector_Weighted_Average_Coefficient[i]
                                                                         * vector_Weighted_Average_Coefficient[i]))
                             * (vector_For_Length_Of_Site[i]) / 2) / 0.38;
        }
        else
        {
            QModelIndex index = model->index(vector_Link_To_Wire[i]->currentIndex(), 0, QModelIndex());
            QVariant resist = model->data(index, Qt::UserRole);
            data_Wire res = resist.value<data_Wire>();
            double activResistMainCore = res.activ_Resistance_Of_The_Main_Core;
            double reactivResistMainCore = res.reactiv_Resistance_Of_The_Main_Core;
            delta_U = (vector_For_Full_Power_Of_Site[i] * (activResistMainCore * vector_Weighted_Average_Coefficient[i]
                             + reactivResistMainCore * sqrt(1 - vector_Weighted_Average_Coefficient[i] * vector_Weighted_Average_Coefficient[i]))
                             * vector_For_Length_Of_Site[i]) / 0.38;
        }

        double delta_U_In_Percents = (delta_U / 380) * 100;
        vector_For_Delta_U.push_back(delta_U);
        vector_For_U_In_Percents.push_back(delta_U_In_Percents);/*вектор для сохранения отклонения напряжения в
                                                                 *процентах*/
    }
    /*************************************************************************************************************/
}
/********************************************************************************************************************/

//Метод расчитывает суммарные отклонения напряжения по участкам
/********************************************************************************************************************/
void MainWindow::calculation_Vector_Of_Sum_Voltage_Deviation()
{
    vector_Sum_For_U_In_Persents.clear();//очистка вектора суммарного отклонения

    //очистка векторов координат
    vector_X1.clear();
    vector_X2.clear();
    vector_Y1.clear();
    vector_Y2.clear();

    double sum = 0;
    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        sum += vector_For_U_In_Percents[i];
        vector_Sum_For_U_In_Persents.push_back(sum);
    }
    vector_Sum_For_U_In_Persents.insert(0, 0);

    for(unsigned int i = 0; i < vector_Sum_For_U_In_Persents.size(); ++i)
    {
        if(vector_Sum_For_U_In_Persents[i] <= 10)
        {
            vector_Y1.push_back(vector_Sum_For_U_In_Persents[i]);
            vector_X1.push_back(i);
        }
        else
        {
            vector_Y2.push_back(vector_Sum_For_U_In_Persents[i]);
            vector_X2.push_back(i);
        }
    }
    vector_Y2.insert(0, vector_Y1[vector_Y1.size()- 1]);
    vector_X2.insert(0, vector_X1[vector_X1.size() - 1]);
}
/********************************************************************************************************************/

//Метод для отрисовки графика отклонения напряжения
/********************************************************************************************************************/
void MainWindow::recruiling_Graphics(QCustomPlot* widg)
{
    //Отрисовка графика отклонения напряжения
    /*************************************************************************************************************/
    widg->clearGraphs();//очистка графиков
    widg->addGraph();//добавляем график №1 в виджет
    widg->addGraph();//добавляем график №2 в виджет
    widg->addGraph();//добавляем график №3 в виджет

    widg->xAxis->setRange(0, number_of_consumers);
    widg->yAxis->setRange(0, *std::max_element(vector_Sum_For_U_In_Persents.begin()
                                                     , vector_Sum_For_U_In_Persents.end()) + 2);

    widg->graph(0)->setData(vector_X1, vector_Y1);
    widg->graph(1)->setData(vector_X2, vector_Y2);
    widg->graph(1)->setPen(QPen(Qt::red));
    QPen pen;
    pen.setStyle(Qt::DashLine);
    QVector <double> dash = {15, 10};
    pen.setDashPattern(dash);
    pen.setColor(QColor(50, 50, 50));
    widg->graph(2)->setPen(pen);
    widg->graph(2)->addData(0, 10);
    widg->graph(2)->addData(number_of_consumers, 10);

    widg->replot();
    /*************************************************************************************************************/
}
/********************************************************************************************************************/

//Слот для расчета нагрузок по участкам
/********************************************************************************************************************/
void MainWindow::slot_Calculation_Of_Loads()
{
    vector_Calculation_Of_Loads.clear();
    vector_For_Full_Power_Of_Site.clear();
    vector_Weighted_Average_Coefficient.clear();
    vector_For_Delta_U.clear();
    vector_For_U_In_Percents.clear();

    slot_Delete_Line_Window_2();
    slot_Delete_Line_Window_4();
    slot_Cleaning_Graphics_Recalculation();

    number = number_of_consumers;//вспомогательная переменная

    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        if(vector_For_Active_Power_Consumer[i] > 300)
        {
            QMessageBox message;
            message.setIcon(QMessageBox::Warning);
            message.setText("Задана нагрузка превышающая 300 кВт!");
            message.exec();

            return;
        }
    }

    //Алгоритм расчета нагрузок на каждом участке линии
    /*************************************************************************************************************/
    for(unsigned int d = 0; d < number_of_consumers; ++d)
    {
        if(difference_Consumers(d))
        {
            //Алгоритм расчета по коэффициенту одновременности
            double Psum = 0;//переменная для сохранения суммы нагрузок
            for(unsigned int k = d; k < number_of_consumers; ++k)
            {
               Psum = Psum + vector_For_Active_Power_Consumer[k];
            }

            double coefficient_Of_Simultaneity = 0;
            switch (ui->comboBox->currentIndex())
            {
                case 0: coefficient_Of_Simultaneity
                        = finding_CoffSimul_and_PowerSuppl(map_Coefficient_Of_Simultaneity_1, number
                                                           ,it_Coeff_Simul_1);
                        --number;
                        break;

                case 1: coefficient_Of_Simultaneity
                        = finding_CoffSimul_and_PowerSuppl(map_Coefficient_Of_Simultaneity_2, number
                                                           , it_Coeff_Simul_2);
                        --number;
                        break;

                case 2: coefficient_Of_Simultaneity
                        = finding_CoffSimul_and_PowerSuppl(map_Coefficient_Of_Simultaneity_3, number
                                                           , it_Coeff_Simul_3);
                        --number;
                        break;

                case 3: coefficient_Of_Simultaneity
                        = finding_CoffSimul_and_PowerSuppl(map_Coefficient_Of_Simultaneity_4, number
                                                           , it_Coeff_Simul_4);
                        --number;
                        break;
            }

            Psum = Psum * coefficient_Of_Simultaneity;
            vector_Calculation_Of_Loads.push_back(Psum);

        }
        else
        {
            //Алгоритм расчета по добавкам мощностей
            double Psum = Pmax;//переменная для сохранения суммарной нагрузки
            for(unsigned int n = d; n < number_of_consumers; ++n)
            {
                if(vector_For_Active_Power_Consumer[n] != Pmax)
                {
                   Psum = Psum + finding_CoffSimul_and_PowerSuppl(map_Power_Supplements
                                                                  , vector_For_Active_Power_Consumer[n]
                                                                  , it_Power_Supplements);
                }
            }
            --number;
            vector_Calculation_Of_Loads.push_back(Psum);
        }
    }
    /*************************************************************************************************************/

    //Расчет полной мощности на участках
    /*************************************************************************************************************/
    calculation_Of_WeightedAverage_Coefficient();

    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
       double d = 0;
       d = vector_Calculation_Of_Loads[i] / vector_Weighted_Average_Coefficient[i];
       vector_For_Full_Power_Of_Site.push_back(d);
    }
    /*************************************************************************************************************/

    //Создание строк для вывода значений активной мощности
    /*************************************************************************************************************/
    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        QHBoxLayout* hBox_5 = new QHBoxLayout();
        QString string_5 = "";
        QLabel* lab_5 = new QLabel("№" + string_5.setNum(i + 1));
        QLineEdit* line_5 = new QLineEdit();
        line_5->setMaximumWidth(80);
        line_5->setReadOnly(true);
        QString str = "";
        line_5->setText(str.setNum(vector_Calculation_Of_Loads[i]));
        hBox_5->addStretch();
        hBox_5->addWidget(lab_5);
        hBox_5->addWidget(line_5);
        hBox_5->addStretch();
        ui->verticalLayout_5->addLayout(hBox_5);
    }

    QWidget* widg_5 = new QWidget(ui->scrollArea_5);
    widg_5->setLayout(ui->verticalLayout_5);
    ui->scrollArea_5->setWidget(widg_5);
    /*************************************************************************************************************/

    //Создание строк для вывода значений полной мощности
    /*************************************************************************************************************/
    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        QHBoxLayout* hBox_6 = new QHBoxLayout();
        QString string_6 = "";
        QLabel* lab_6 = new QLabel("№" + string_6.setNum(i + 1));
        QLineEdit* line_6 = new QLineEdit();
        line_6->setMaximumWidth(80);
        line_6->setReadOnly(true);
        QString str_6 = "";
        line_6->setText(str_6.setNum(vector_For_Full_Power_Of_Site[i]));
        hBox_6->addStretch();
        hBox_6->addWidget(lab_6);
        hBox_6->addWidget(line_6);
        hBox_6->addStretch();
        ui->verticalLayout_6->addLayout(hBox_6);
    }

    QWidget* widg_6 = new QWidget(ui->scrollArea_6);
    widg_6->setLayout(ui->verticalLayout_6);
    ui->scrollArea_6->setWidget(widg_6);
    /*************************************************************************************************************/

    //Создание строк для вывода значений средневзвешенных коэффициентов
    /*************************************************************************************************************/
    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        QHBoxLayout* hBox_7 = new QHBoxLayout();
        QString string_7 = "";
        QLabel* lab_7 = new QLabel("№" + string_7.setNum(i + 1));
        QLineEdit* line_7 = new QLineEdit();
        line_7->setMaximumWidth(80);
        line_7->setReadOnly(true);
        QString str_7 = "";
        line_7->setText(str_7.setNum(vector_Weighted_Average_Coefficient[i]));
        hBox_7->addStretch();
        hBox_7->addWidget(lab_7);
        hBox_7->addWidget(line_7);
        hBox_7->addStretch();
        ui->verticalLayout_7->addLayout(hBox_7);

        QWidget* widg_7 = new QWidget(ui->scrollArea_7);
        widg_7->setLayout(ui->verticalLayout_7);
        ui->scrollArea_7->setWidget(widg_7);
    }
    /*************************************************************************************************************/

    calculation_Of_Voltage_Deviation();//расчет отклонения напряжения

    //Создание строк для вывода значений отклонений напряжений на участках
    /*************************************************************************************************************/
    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        QHBoxLayout* hBox_8 = new QHBoxLayout();
        QString string_8 = "";
        QLabel* lab_8 = new QLabel("№" + string_8.setNum(i + 1));
        QLineEdit* line_8 = new QLineEdit();
        line_8->setMaximumWidth(100);
        line_8->setReadOnly(true);
        QString str_8 = "";
        QString str = "";
        line_8->setText(str_8.setNum(vector_For_Delta_U[i]) + " (" + str.setNum(vector_For_U_In_Percents[i])
                        + ")");
        hBox_8->addStretch();
        hBox_8->addWidget(lab_8);
        hBox_8->addWidget(line_8);
        hBox_8->addStretch();
        ui->verticalLayout_8->addLayout(hBox_8);
    }

    QWidget* widg_8 = new QWidget(ui->scrollArea_8);
    widg_8->setLayout(ui->verticalLayout_8);
    ui->scrollArea_8->setWidget(widg_8);
    /*************************************************************************************************************/

    //Создание строк для вывода значений суммарного отклонения напряжения на участках
    /*************************************************************************************************************/
     double sum = 0;
    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        QHBoxLayout* hBox_13 = new QHBoxLayout();
        QString string_13 = "";
        QLabel* lab_13 = new QLabel("№" + string_13.setNum(i + 1));
        QLineEdit* line_13 = new QLineEdit();
        line_13->setMaximumWidth(80);
        line_13->setReadOnly(true);
        QString str_13 = "";

        sum += vector_For_U_In_Percents[i];

        line_13->setText(str_13.setNum(sum));
        hBox_13->addStretch();
        hBox_13->addWidget(lab_13);
        hBox_13->addWidget(line_13);
        hBox_13->addStretch();
        ui->verticalLayout_13->addLayout(hBox_13);

        QWidget* widg_13 = new QWidget(ui->scrollArea_13);
        widg_13->setLayout(ui->verticalLayout_13);
        ui->scrollArea_13->setWidget(widg_13);
    }
    /*************************************************************************************************************/
    ui->tabWidget->setCurrentWidget(ui->tab_2);//установка текущей активной вкладки

    calculation_Vector_Of_Sum_Voltage_Deviation();//расчет суммарных отклонений по участкам

    recruiling_Graphics(ui->widget);//отрисовка начального графика

    if(!(ui->checkBox->isChecked()))
    {
        if(ui->radioButton->isChecked())
        {
            voltage_Check_2();//проверка отклонения напряжения методот от большего к меньшему
        }
        else
        {
            voltage_Check();//проверка отклонения по максимальному отклонению
        }


        recruiling_Graphics(ui->widget_2);//отрисовка графика перерасчета

        data_Recalculation();

        //Возвращение индексов в прежнее положение
        QList <QComboBox*> listComboBox = ui->scrollArea_4->findChildren <QComboBox*> ();
        for(unsigned int i = 0; i < number_of_consumers; ++i)
        {
            listComboBox[i]->setCurrentIndex(vec_Return_Index_For_ComboBox[i]);
        }
    }
}
/********************************************************************************************************************/

//Метод проверки величины отклонения напряжения
/********************************************************************************************************************/
void MainWindow::voltage_Check()
{
    //Проверка допустимости величины отклонения напряжения
    QVector <double> vec_Temp = vector_For_U_In_Percents;
    double temp_Double;
    if(vector_Sum_For_U_In_Persents[number_of_consumers] > 10)
    {
        //проверка на равенство нулю всех элементов вектора bool_Vec_Temp
        bool b_Element = true;
        for(auto el: bool_Vec_Temp)
        {
            if(el != false) b_Element = false;
        }

        //Если в vec_Temp все элементы не равны нулю, производим расчет, иначе выводим сообщение
        if(!b_Element)
        {
            temp_Double = *std::max_element(vec_Temp.begin(), vec_Temp.end());
            temp = vec_Temp.indexOf(temp_Double);

            if((vec_Pair_Index[temp].first == 9 && vec_Pair_Index[temp].second == 9)
                    && vector_Link_To_Wire[temp]->currentIndex() == 9)
            {
                bool_Vec_Temp[temp] = 0;
                vec_Temp[temp] = 0;
            }

            for(unsigned int i = 0; i < number_of_consumers; ++i)
            {
                if(bool_Vec_Temp[i] == 0)
                {
                    vec_Temp[i] = 0;
                }
            }

            temp_Double = *std::max_element(vec_Temp.begin(), vec_Temp.end());
            temp = vec_Temp.indexOf(temp_Double);

            if((vec_Pair_Index[temp].first == 9 && vec_Pair_Index[temp].second == 9)
                    && vector_Link_To_Wire[temp]->currentIndex() == 9)
            {
                bool_Vec_Temp[temp] = 0;
                vec_Temp[temp] = 0;
            }

            //Проверяем длину участка, если она больше 0,8 км делим длину пополам и добавляем один участок
            if(vector_For_Length_Of_Site[temp] >= 0.8)
            {
                if(vec_Bool_For_Pair[temp])
                {
                    if(vec_Pair_Index[temp].first < 9)
                    {
                        ++vec_Pair_Index[temp].first;
                    }
                    vector_Link_To_Wire[temp]->setCurrentIndex(vector_Link_To_Wire[temp]->currentIndex() + 1);
                    vec_Bool_For_Pair[temp] = false;
                }
                else
                {
                    if(vec_Pair_Index[temp].second < 9)
                    {
                       ++vec_Pair_Index[temp].second;
                    }
                    vec_Bool_For_Pair[temp] = true;
                }
            }
            else
            {
               //Повышение индекса ComboBox участка с наибольшей потерей напряжения
               if(vector_Link_To_Wire[temp]->currentIndex() < 9)
               {
                  vector_Link_To_Wire[temp]->setCurrentIndex(vector_Link_To_Wire[temp]->currentIndex() + 1);
               }
               else
               {
                   bool_Vec_Temp[temp] = 0;
                   vec_Temp[temp] = 0;

                   //Проверка на равенство нулю всех элементов вектора bool_Vec_Temp
                   bool d = false;
                   for(auto b : bool_Vec_Temp)
                   {
                       if(b == true)
                       {
                           d = b;
                           break;
                       }
                   }

                   if(d == false)
                   {
                       QMessageBox message;
                       message.setText("На всех участках максимально допустимое сечение!");
                       message.exec();

                       flag_Message = true;

                       return;
                   }
                   else
                   {
                        voltage_Check();
                   }
               }
            }         

            calculation_Of_Voltage_Deviation();
            calculation_Vector_Of_Sum_Voltage_Deviation();
            voltage_Check();
        }
        else
        {
            if(!flag_Message)
            {
                QMessageBox message;
                message.setIcon(QMessageBox::Warning);
                message.setText("На всех участках максимально допустимое сечение!");
                message.exec();
                flag_Message = true;
                return;
            }
        }
    }
}
/********************************************************************************************************************/

//Метод проверки величины отклонения напряжения
/********************************************************************************************************************/
void MainWindow::voltage_Check_2()
{
    QVector <double> vec_Temp = vector_For_U_In_Percents;//сохранряем значения отклонения в процентах во временный массив
    double temp_Double;//временная переменная для сохраненя величины максимального отклонения
    if(vector_Sum_For_U_In_Persents[number_of_consumers] > 10)
    {
        //Заполняем нулями участки где уже максимальное сечение провода
        unsigned int nullPositionsEnd = 0;//задает индекс в векторе vec_Temp с которого начинаются не нулевые значения
        bool flag = true;
        for(unsigned int i = 0; i < number_of_consumers; ++i)
        {
            if(vector_For_Length_Of_Site[i] >= 0.8)
            {
                if(vec_Pair_Index[i].first == 9 && vec_Pair_Index[i].second == 9) vec_Temp[i] = 0;
            }
            else
            {
                if(bool_Vec_Temp[i] == 0) vec_Temp[i] = 0;
            }
        }

        //Находим максимальное отклонение
        temp_Double = *std::max_element(vec_Temp.begin(), vec_Temp.end());
        temp = vec_Temp.indexOf(temp_Double);//находим индекс элемента с максимальным отклонением

        //Если участок составной, то
        //находим половину участка с наименьшим сечением, т.к. на нем будут максимальные потери
        int& smallerPairSite = vec_Pair_Index[temp].first <= vec_Pair_Index[temp].second?
                    vec_Pair_Index[temp].first : vec_Pair_Index[temp].second;

        //Ищем первый не нулевой участок
        for(unsigned int i = 0; i <= temp; ++i)
        {
            if(vec_Temp[i] && flag)
            {
                 nullPositionsEnd = i;
                 flag =false;
            }
        }

        //Проверка на равенство нулю всех элементов вектора bool_Vec_Temp
        bool b_Element = true;
        for(auto el: bool_Vec_Temp)
        {
            if(el != false) b_Element = false;
        }

        //Если в vec_Temp все элементы не равны нулю, производим расчет, иначе выводим сообщение
        if(!b_Element)
        {
            int cutIndexMax = 0;//сохраняет максимальный индекс сечения
            int sectorMax = nullPositionsEnd;//сохраняет номер участка с максимальным сечением
            int sectorMin = nullPositionsEnd;;//сохраняет индекс участка с минимальным сечением
            int cutIndexMin = 9;//сохраняет минимальный индекс сечения
            int boolSectorMax = -1;
            int boolSectorMin = -1;

            //Находим на каком участке от первого до участка с индексом temp максимальное сечение провода
            /*********************************************************************************************************/
            for(unsigned int i = nullPositionsEnd; i <= temp; ++i)
            {
                if(vector_For_Length_Of_Site[i] >= 0.8)
                {
                    if(i == temp)
                    {
                        if(vec_Pair_Index[i].first != smallerPairSite)
                        {
                            if(vec_Pair_Index[i].first > cutIndexMax && vec_Pair_Index[i].first != -1)
                            {
                                cutIndexMax = vec_Pair_Index[i].first;
                                sectorMax = i;
                                boolSectorMax = 0;
                            }
                        }
                        else break;

                        if(vec_Pair_Index[i].second > cutIndexMax && vec_Pair_Index[i].second != -1)
                        {
                            cutIndexMax = vec_Pair_Index[i].second;
                            sectorMax = i;
                            boolSectorMax = 1;
                        }
                    }
                    else
                    {
                        if(vec_Pair_Index[i].second > cutIndexMax && vec_Pair_Index[i].second != -1)
                        {
                            cutIndexMax = vec_Pair_Index[i].second;
                            sectorMax = i;
                            boolSectorMax = 1;
                        }
                        if(vec_Pair_Index[i].first > cutIndexMax && vec_Pair_Index[i].first != -1)
                        {
                            cutIndexMax = vec_Pair_Index[i].first;
                            sectorMax = i;
                            boolSectorMax = 0;
                        }
                    }
                }
                else
                {
                    if(i != temp)
                    {
                        if(vector_Link_To_Wire[i]->currentIndex() > cutIndexMax && vec_Temp[i] != 0)
                        {
                            sectorMax = i;
                        }
                    }
                }
            }
            /*********************************************************************************************************/

            /*********************************************************************************************************/
            //Создаем лямда-выражение для использования вложенной функции
            /*auto findMinCutSector
            {
                [cutIndexMax, sectorMax, &vec_Temp](int begin, int end)
                {

                }
            };*/
            /*********************************************************************************************************/

            //Если sectorMin > 0
            /*********************************************************************************************************/
            if(sectorMax != 0)
            {
                //Вычисляем участок с наименьшим сечением на участке от 0 до sectorMax
                for(int i = nullPositionsEnd; i <= sectorMax; ++i)
                {
                    if(vector_For_Length_Of_Site[i] >= 0.8)
                    {
                        if(vec_Pair_Index[i].second < cutIndexMax && vec_Pair_Index[i].second != -1)
                        {
                            cutIndexMin = vec_Pair_Index[i].second;
                            sectorMin = i;
                            boolSectorMin = 1;
                        }
                        if(vec_Pair_Index[i].first < cutIndexMax && vec_Pair_Index[i].first != -1)
                        {
                            cutIndexMin = vec_Pair_Index[i].first;
                            sectorMin = i;
                            boolSectorMin = 0;
                        }
                    }
                    else
                    {
                        //Если участок одинарный, то когда i == sectorMax не надо учитывать
                        if(i != sectorMax)
                        {
                            if(vector_Link_To_Wire[i]->currentIndex() < cutIndexMax && vec_Temp[i] != 0)
                            {
                                sectorMin = i;
                            }
                        }
                    }
                }

                //Выбирается один из половины составного участка sectorMin с наименьшим сечением
                int& indexPairSite = boolSectorMin? vec_Pair_Index[sectorMin].second : vec_Pair_Index[sectorMin].first;
                //Выбор меньшего из участков, если temp составной
                //int& smallerPairSite = vec_Pair_Index[temp].first <= vec_Pair_Index[temp].second?
                            //vec_Pair_Index[temp].first : vec_Pair_Index[temp].second;
                //Выбор составной участок temp или одинарный
                const int& compositeSite = vector_For_Length_Of_Site[temp] >= 0.8?
                            smallerPairSite: vector_Link_To_Wire[temp]->currentIndex();

                //Если участок sectorMin составной
                if(vector_For_Length_Of_Site[sectorMin] >= 0.8)
                {
                    /*Если сечение на участке с минимальным сечением больше чем на участке temp то повышаем сечение
                     *на участке temp*/
                    if(indexPairSite > compositeSite)//от sectorMin до temp
                    {
                        //Если участок temp составной, то повышаем сечение половины, у которой меньше сечение
                        if(vector_For_Length_Of_Site[temp] >= 0.8)
                        {
                            int curInd = compositeSite;
                            ++curInd;
                            smallerPairSite = curInd;
                            //Убираем из расчетов участок, если на нем максимальное сечение
                            //if(curInd == 9)smallerPairSite = -1;
                        }
                        //Иначе повышаем сечение полного участка temp
                        else
                        {
                            vector_Link_To_Wire[temp]->setCurrentIndex(vector_Link_To_Wire[temp]->currentIndex() + 1);
                            //Убираем из расчетов участок, если на нем максимальное сечение
                            if(vector_Link_To_Wire[temp]->currentIndex() == 9) bool_Vec_Temp[temp] = 0;
                        }
                    }
                    //Иначе повышаем сечение на участке sectorMin
                    else
                    {
                        ++indexPairSite;
                        //Убираем из расчетов участок, если на нем максимальное сечение
                        //if(indexPairSite == 9) indexPairSite = -1;
                    }
                }
                //Иначе, если участок sectorMin одинарный
                else
                {
                    /*Если сечение на участке с минимальным сечением больше чем на участке temp то повышаем сечение
                     *на участке temp*/
                    if(vector_Link_To_Wire[sectorMin]->currentIndex() > compositeSite)
                    {
                        //Если участок temp составной, то повышаем сечение половины, у которой меньше сечение
                        if(vector_For_Length_Of_Site[temp] >= 0.8)
                        {
                            int curInd = compositeSite;
                            ++curInd;
                            smallerPairSite = curInd;
                            //Убираем из расчетов участок, если на нем максимальное сечение
                            //if(curInd == 9)smallerPairSite = -1;
                        }
                        //Иначе повышаем сечение полного участка temp
                        else
                        {
                            vector_Link_To_Wire[temp]->setCurrentIndex(vector_Link_To_Wire[temp]->currentIndex() + 1);
                            //Убираем из расчетов участок, если на нем максимальное сечение
                            //if(vector_Link_To_Wire[temp]->currentIndex() == 9) bool_Vec_Temp[temp] = 0;
                        }
                    }
                    //Иначе повышаем сечение на участке sectorMin
                    else
                    {
                        vector_Link_To_Wire[sectorMin]->setCurrentIndex(vector_Link_To_Wire[sectorMin]->currentIndex() + 1);
                        if(vector_Link_To_Wire[sectorMin]->currentIndex() == 9) bool_Vec_Temp[sectorMin] = 0;
                    }
                }
            }
            //Если sectorMin == 0
            /*********************************************************************************************************/
            else
            {
                //Вычисляем участок с наименьшим сечением на участке от sectorMin до temp
                for(unsigned int i = sectorMax; i < temp; ++i)
                {
                    if(vector_For_Length_Of_Site[i] >= 0.8)
                    {
                        if(vec_Pair_Index[i].second < cutIndexMax && vec_Pair_Index[i].second != -1)
                        {
                            cutIndexMin = vec_Pair_Index[i].second;
                            sectorMin = i;
                            boolSectorMin = 1;
                        }
                        if(vec_Pair_Index[i].first < cutIndexMax && vec_Pair_Index[i].first != -1)
                        {
                            cutIndexMin = vec_Pair_Index[i].first;
                            sectorMin = i;
                            boolSectorMin = 0;
                        }
                    }
                    else
                    {
                        if(i != temp)
                        {
                            if(vector_Link_To_Wire[i]->currentIndex() < cutIndexMax && vec_Temp[i] != 0)
                            {
                                sectorMin = i;
                            }
                        }
                    }
                }


                //Выбирается один из половины составного участка sectorMin с наименьшим сечением
                int& indexPairSite = boolSectorMin? vec_Pair_Index[sectorMin].second : vec_Pair_Index[sectorMin].first;
                //Выбор меньшего из участков, если temp составной
                //int& smallerPairSite = vec_Pair_Index[temp].first <= vec_Pair_Index[temp].second?
                            //vec_Pair_Index[temp].first : vec_Pair_Index[temp].second;
                //Выбор составной участок temp или одинарный
                const int& compositeSite = vector_For_Length_Of_Site[temp] >= 0.8?
                            smallerPairSite: vector_Link_To_Wire[temp]->currentIndex();

                //Если участок sectorMin составной
                if(vector_For_Length_Of_Site[sectorMin] >= 0.8)
                {
                    /*Если сечение на участке с минимальным сечением больше чем на участке temp то повышаем сечение
                     *на участке temp*/
                    if(indexPairSite > compositeSite)
                    {
                        //Если участок temp составной, то повышаем сечение половины, у которой меньше сечение
                        if(vector_For_Length_Of_Site[temp] >= 0.8)
                        {
                            int curInd = compositeSite;
                            ++curInd;
                            smallerPairSite = curInd;
                            //Убираем из расчетов участок, если на нем максимальное сечение
                            //if(curInd == 9)smallerPairSite = -1;
                        }
                        //Иначе повышаем сечение полного участка temp
                        else
                        {
                            vector_Link_To_Wire[temp]->setCurrentIndex(vector_Link_To_Wire[temp]->currentIndex() + 1);
                            //Убираем из расчетов участок, если на нем максимальное сечение
                            if(vector_Link_To_Wire[temp]->currentIndex() == 9) bool_Vec_Temp[temp] = 0;
                        }
                    }
                    //Иначе повышаем сечение на участке sectorMin
                    else
                    {
                        ++indexPairSite;
                        //Убираем из расчетов участок, если на нем максимальное сечение
                        //if(indexPairSite == 9) indexPairSite = -1;
                    }
                }
                //Иначе, если участок sectorMin одинарный
                else
                {
                    /*Если сечение на участке с минимальным сечением больше чем на участке temp то повышаем сечение
                     *на участке temp*/
                     if(vector_Link_To_Wire[sectorMin]->currentIndex() > compositeSite)
                    {
                        //Если участок temp составной, то повышаем сечение половины, у которой меньше сечение
                        if(vector_For_Length_Of_Site[temp] >= 0.8)
                        {
                            int curInd = compositeSite;
                            ++curInd;
                            smallerPairSite = curInd;
                            //Убираем из расчетов участок, если на нем максимальное сечение
                            //if(curInd == 9)smallerPairSite = -1;
                        }
                        //Иначе повышаем сечение полного участка temp
                        else
                        {
                            vector_Link_To_Wire[temp]->setCurrentIndex(vector_Link_To_Wire[temp]->currentIndex() + 1);
                            //Убираем из расчетов участок, если на нем максимальное сечение
                            if(vector_Link_To_Wire[temp]->currentIndex() == 9) bool_Vec_Temp[temp] = 0;
                        }
                    }
                    //Иначе повышаем сечение на участке sectorMin
                    else
                    {
                        vector_Link_To_Wire[sectorMin]->setCurrentIndex(vector_Link_To_Wire[sectorMin]->currentIndex() + 1);
                        if(vector_Link_To_Wire[sectorMin]->currentIndex() == 9) bool_Vec_Temp[sectorMin] = 0;
                    }
                }
            }

            calculation_Of_Voltage_Deviation();
            calculation_Vector_Of_Sum_Voltage_Deviation();
            voltage_Check_2();
        }
        else
        {
            QMessageBox message;
            message.setIcon(QMessageBox::Warning);
            message.setText("На всех участках максимально допустимое сечение!");
            message.exec();
            flag_Message = true;
            return;
        }
    }
}
/********************************************************************************************************************/

//Установка данных в окне данных перерасчета
/********************************************************************************************************************/
void MainWindow::data_Recalculation()
{
    for(unsigned int i = 0; i < number_of_consumers; ++i)
    {
        //Установка строк с данными для окна сечения провода
        /***********************************************************************************************************/
        ui->verticalLayout_9->setSpacing(2);
        QHBoxLayout* hBox_9_1 = new QHBoxLayout();
        QHBoxLayout* hBox_9_2 = new QHBoxLayout();
        QString string_9 = "";
        QLabel* lab_9 = new QLabel("№" + string_9.setNum(i + 1));
        QLineEdit* line_9_1 = new QLineEdit();
        line_9_1->setMaximumWidth(110);
        line_9_1->setReadOnly(true);
        QLineEdit* line_9_2 = new QLineEdit();
        line_9_2->setMaximumWidth(110);
        line_9_2->setReadOnly(true);

        if(vector_For_Length_Of_Site[i] >= 0.8)
        {
           QModelIndex index_9_1 = model->index(vec_Pair_Index[i].first, 0, QModelIndex());
           QVariant name_9_1 = model->data(index_9_1, Qt::DisplayRole);
           QModelIndex index_9_2 = model->index(vec_Pair_Index[i].second, 0, QModelIndex());
           QVariant name_9_2 = model->data(index_9_2, Qt::DisplayRole);
           line_9_1->setText(name_9_1.toString());
           line_9_2->setText(name_9_2.toString());
           hBox_9_1->addStretch();
           hBox_9_1->addWidget(lab_9);
           hBox_9_1->addWidget(line_9_1);
           hBox_9_1->addStretch();

           hBox_9_2->addSpacing(30);
           hBox_9_2->addWidget(line_9_2);
           hBox_9_2->addStretch();

           QVBoxLayout* vBox = new QVBoxLayout();
           vBox->addLayout(hBox_9_1);
           vBox->addLayout(hBox_9_2);

           ui->verticalLayout_9->addLayout(vBox);
        }
        else
        {
            QModelIndex index_9 = model->index(vector_Link_To_Wire[i]->currentIndex(), 0, QModelIndex());
            QVariant name_9 = model->data(index_9, Qt::DisplayRole);
            line_9_1->setText(name_9.toString());
            hBox_9_1->addStretch();
            hBox_9_1->addWidget(lab_9);
            hBox_9_1->addWidget(line_9_1);
            hBox_9_1->addStretch();

            ui->verticalLayout_9->addLayout(hBox_9_1);
        }

        QWidget* widg_9 = new QWidget(ui->scrollArea_9);
        widg_9->setLayout(ui->verticalLayout_9);
        ui->scrollArea_9->setWidget(widg_9);
        /***********************************************************************************************************/

        //Установка строк с данными для окна потери напряжения
        /***********************************************************************************************************/
        QHBoxLayout* hBox_10 = new QHBoxLayout();
        QString string_10 = "";
        QLabel* lab_10 = new QLabel("№" + string_10.setNum(i + 1));
        QLineEdit* line_10 = new QLineEdit();
        line_10->setMaximumWidth(110);
        line_10->setReadOnly(true);
        QString txt_10 = "";
        line_10->setText(txt_10.setNum(vector_For_Delta_U[i]));
        hBox_10->addStretch();
        hBox_10->addWidget(lab_10);
        hBox_10->addWidget(line_10);
        hBox_10->addStretch();
        ui->verticalLayout_10->addLayout(hBox_10);

        QWidget* widg_10 = new QWidget(ui->scrollArea_10);
        widg_10->setLayout(ui->verticalLayout_10);
        ui->scrollArea_10->setWidget(widg_10);
        /***********************************************************************************************************/

        //Установка строк с данными для окна потери напряжения в %
        /***********************************************************************************************************/
        QHBoxLayout* hBox_11 = new QHBoxLayout();
        QString string_11 = "";
        QLabel* lab_11 = new QLabel("№" + string_11.setNum(i + 1));
        QLineEdit* line_11 = new QLineEdit();
        line_11->setMaximumWidth(110);
        line_11->setReadOnly(true);
        QString txt_11 = "";
        line_11->setText(txt_11.setNum(vector_For_U_In_Percents[i]));
        hBox_11->addStretch();
        hBox_11->addWidget(lab_11);
        hBox_11->addWidget(line_11);
        hBox_11->addStretch();
        ui->verticalLayout_11->addLayout(hBox_11);

        QWidget* widg_11 = new QWidget(ui->scrollArea_11);
        widg_11->setLayout(ui->verticalLayout_11);
        ui->scrollArea_11->setWidget(widg_11);
        /***********************************************************************************************************/

        //Установка строк с данными для окна суммарные потери напряжения в %
        /***********************************************************************************************************/
        QHBoxLayout* hBox_12 = new QHBoxLayout();
        QString string_12 = "";
        QLabel* lab_12 = new QLabel("№" + string_12.setNum(i + 1));
        QLineEdit* line_12 = new QLineEdit();
        line_12->setMaximumWidth(110);
        line_12->setReadOnly(true);
        QString txt_12 = "";
        line_12->setText(txt_12.setNum(vector_Sum_For_U_In_Persents[i + 1]));
        hBox_12->addStretch();
        hBox_12->addWidget(lab_12);
        hBox_12->addWidget(line_12);
        hBox_12->addStretch();
        ui->verticalLayout_12->addLayout(hBox_12);

        QWidget* widg_12 = new QWidget(ui->scrollArea_12);
        widg_12->setLayout(ui->verticalLayout_12);
        ui->scrollArea_12->setWidget(widg_12);
        /***********************************************************************************************************/
    }
}
/********************************************************************************************************************/

//Слот для обнуления графиков
/********************************************************************************************************************/
void MainWindow::slot_Cleaning_Graphics_Recalculation()
{
    ui->widget->clearGraphs();
    ui->widget->replot();

    ui->widget_2->clearGraphs();
    ui->widget_2->replot();
}
/********************************************************************************************************************/

//Слот для обнуления окна перерасчета
/********************************************************************************************************************/
void MainWindow::slot_Delete_Line_Window_4()
{
    //Удаление строк в окне вывода сечений проводов
    /****************************************************************************************************************/
    QList <QLabel*> labList_9 = ui->scrollArea_9->findChildren <QLabel*> ();
    foreach(QLabel* lab_9, labList_9)
    {
        delete lab_9;
    }

    QList <QLineEdit*> lineList_9 = ui->scrollArea_9->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_9, lineList_9)
    {
        delete line_9;
    }
    /****************************************************************************************************************/

    //Удаление строк в окне потерь напряжения
    /****************************************************************************************************************/
    QList <QLabel*> labList_10 = ui->scrollArea_10->findChildren <QLabel*> ();
    foreach(QLabel* lab_10, labList_10)
    {
        delete lab_10;
    }

    QList <QLineEdit*> lineList_10 = ui->scrollArea_10->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_10, lineList_10)
    {
        delete line_10;
    }
    /****************************************************************************************************************/

    //Удаление строк в окне потерь напряжения в %
    /****************************************************************************************************************/
    QList <QLabel*> labList_11 = ui->scrollArea_11->findChildren <QLabel*> ();
    foreach(QLabel* lab_11, labList_11)
    {
        delete lab_11;
    }

    QList <QLineEdit*> lineList_11 = ui->scrollArea_11->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_11, lineList_11)
    {
        delete line_11;
    }
    /****************************************************************************************************************/

    //Удаление строк в окне суммарных потерь
    /****************************************************************************************************************/
    QList <QLabel*> labList_12 = ui->scrollArea_12->findChildren <QLabel*> ();
    foreach(QLabel* lab_12, labList_12)
    {
        delete lab_12;
    }

    QList <QLineEdit*> lineList_12 = ui->scrollArea_12->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_12, lineList_12)
    {
        delete line_12;
    }
    /****************************************************************************************************************/
}
/********************************************************************************************************************/

//Слот для установки валидатора для строк ввода активной мощности
/********************************************************************************************************************/
void MainWindow::slot_Set_Validator_For_Activ_Power()
{
    QList <QLineEdit*> lineList_Valid = ui->scrollArea->findChildren <QLineEdit*> ();
    foreach(QLineEdit* line_Valid, lineList_Valid)
    {
       line_Valid->setText("");//обнуление строк ввода активной мощности
       if(ui->comboBox->currentIndex() == 0)
       {
           QRegularExpression reg_Valid("[0-1]{1}\\.{1,1}[0-9]+|[0-2]{1}");
           QValidator* valid = new QRegularExpressionValidator(reg_Valid, line_Valid);
           line_Valid->setValidator(valid);
       }
       else
       {
           QRegularExpression reg_Valid("\\d+\\.?[0-9]+");
           QValidator* valid = new QRegularExpressionValidator(reg_Valid, line_Valid);
           line_Valid->setValidator(valid);
       }
    }
}
/********************************************************************************************************************/
