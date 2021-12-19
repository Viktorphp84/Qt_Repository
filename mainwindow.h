#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>

class QComboBox;
class QCustomPlot;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

//Сопротивления провода
/********************************************************************************************************************/
struct data_Wire
{
    double activ_Resistance_Of_The_Main_Core;  //активное сопротивление основной жилы
    double activ_Resistance_Of_The_Zero_Core;  //активное сопротивление нулевой жилы
    double reactiv_Resistance_Of_The_Main_Core;//реактивное сопротивление основной жилы
    double reactiv_Resistance_Of_The_Zero_Core;//реактивное сопротивление нулевой жилы
    double section_Of_Wire;//сечение основной жилы
};
/********************************************************************************************************************/

//Структура для сохранения значений сопротивлений проводов
/********************************************************************************************************************/
struct the_Wire
{
   QString name;                              //название провода
   data_Wire data_Wire;
};
/********************************************************************************************************************/

/********************************************************************************************************************/
class ComboBoxModel: public QAbstractListModel
{
    Q_OBJECT
public:
    ComboBoxModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex&) const;
    QVariant data(const QModelIndex& index, int role) const;
    void populate(QList<the_Wire>* newValues);
private:
    QList<the_Wire>* values;
};
/********************************************************************************************************************/

/********************************************************************************************************************/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
   void calculation_Of_WeightedAverage_Coefficient();//функция для расчета средневзвешенных коэффициентов по участкам
   bool difference_Consumers(unsigned int);/*функция для расчета отличия нагрузок возвращает true,
                                             если нагрузки отличаются менее или в 4 раза*/
   double Pmax = 0;//переменная для сохранения значения максимальной нагрузки на текущем участке

   void voltage_Check();//метод для проверки отклонения напряжения
   void voltage_Check_2();//метод для проверки отклонения напряжения с повышением сечения предыдущих участков
   void calculation_Of_Voltage_Deviation();//метод для расчета отклонения напряжения
   void recruiling_Graphics(QCustomPlot*);//метод для отрисовки графика
   void calculation_Vector_Of_Sum_Voltage_Deviation();//метод расчитывает суумарные отклонения напряжения по участкам
   void data_Recalculation();//метод для установки строк с данными в окне перерасчета

   template<typename T1, typename T2, typename T3>
   double finding_CoffSimul_and_PowerSuppl(T1&, T2&, T3);//функция для расчета коэффициентов одновременности и добавок

   unsigned int number_of_consumers;//число нагрузок
   unsigned int number;//вспомогательная переменная для расчета коэф. одновременности и добавок
   unsigned int current_Type_Of_Load;//текущий тип нагрузки
   unsigned int current_Index_ComboBox;//переменная сохраняет текущий индекс ComboBox, в котором изменяется сечение
   QComboBox* current_ComboBox;//указатель на ComboBox, в котором изменяется сечение
   bool Bool = true;//для сохранения current_Index_ComboBox и current_ComboBox только один раз в блоке
   unsigned int index_1 = 0;/*переменная для сохранения индекса сечения первой половины провода,
                             *если длина участка больше 0,8 км*/
   unsigned int index_2 = 0;/*переменная для сохранения индекса сечения второй половины провода,
                             *если длина участка больше 0,8 км*/
   double half_Length = 0;//половина длины участка боьше или равного 0,8 км
   bool bool_Index = false;/*при разделении участка более 0,8 км на две части, переменная указывает на какой из
                            *них следует поднять сечение*/
   bool calculation_Two_Site = false;/*если true то при расчете отклонения напряжения учитывается деление участка более
                                     *0,8 км на две части*/
   bool flag_Message = false;//флаг показывает выводилось ли сообщение о максимальном сечении на участках или нет
   bool flag_For_Line = false;//флаг показывает заполнены ли все данные или есть пустые строки
   unsigned int temp = 0;/*сохраняет индекс максимального элемента при расчете величины отклонения напряжения в методе
                      *voltage_Check()*/

   QVector <double> vector_Weighted_Average_Coefficient;//вектор для сохранения средневзвешенных коэффициентов по участкам
   QVector <double> vector_For_Active_Power_Consumer;//вектор для сохранения активных мощностей потребителей
   QVector <double> vector_For_Length_Of_Site;//вектор для сохранения длин участков
   QVector <double> vector_For_Cofficient_Of_Active_Power;//вектор для сохранения коэффициентов ативной мощности
   //QVector <double> vector_For_Section_Of_Wire;//вектор для сохранения сечений проводов
   QVector <double> vector_Calculation_Of_Loads;//вектор для сохранения расчитанных нагрузок по учаткам
   QVector <double> vector_For_Full_Power_Of_Site;//вектор для сохранения полной мощности на участках
   QVector <double> vector_For_Delta_U;//вектор для сохранения значений отклонения напряжения на учасках
   QVector <double> vector_For_U_In_Percents;//вектор для сохранения значений отклонения напряжения в процентах
   QVector <QComboBox*> vector_Link_To_Wire;//вектор сохраняет указатели на строки comboBox
   QVector <double> vector_Sum_For_U_In_Persents;/*векор сохраняет в ячейку сумму предыдущего и текущего элементов из вектора
                                                  *отклонения напряжения в процетах*/
   QVector <double> vector_X1;//сохраняет координаты точек по оси X до 10%
   QVector <double> vector_Y1;//сохраняет координаты точек по оси Y до 10%
   QVector <double> vector_X2;//сохраняет координаты точек по оси X после 10%
   QVector <double> vector_Y2;//сохраняет координаты точек по оси Y после 10%
   QVector <bool> bool_Vec_Temp;/*отмечает нулями участки на которых уже установлены максимально
                                 *возможные индексы для сечений*/
   QVector <QPair<int, int>> vec_Pair_Index;//вектор хранит пары индексов участков более 0,8 км для расчетов
   QVector <bool> vec_Bool_For_Pair;/*вектор сохраняет флаг состояния для каждого участка (т.е. определяет первую
                                     *или вторую часть участка нужно увеличить по сечению)*/

   //Коэффициенты одновременности для жилых домов с нагрузкой до 2 кВт
   QMap <unsigned int, double> map_Coefficient_Of_Simultaneity_1 = {
                                                                    {1, 1},     //1
                                                                    {2, 0.76},  //2
                                                                    {3, 0.66},  //2
                                                                    {5, 0.55},  //4
                                                                    {10, 0.44}, //5
                                                                    {20, 0.37}, //6
                                                                    {50, 0.30}, //7
                                                                    {100, 0.26},//8
                                                                    {200, 0.24},//9
                                                                    {500, 0.22},//10
                                                                   };
   QMap <unsigned int, double>::iterator it_Coeff_Simul_1 = map_Coefficient_Of_Simultaneity_1.begin();

   //Коэффициенты одновременности для жилых домов с нагрузкой свыше 2 кВт
   QMap <unsigned int, double> map_Coefficient_Of_Simultaneity_2 = {
                                                                    {1, 1},     //1
                                                                    {2, 0.75},  //2
                                                                    {3, 0.64},  //3
                                                                    {5, 0.53},  //4
                                                                    {10, 0.42}, //5
                                                                    {20, 0.34}, //6
                                                                    {50, 0.27}, //7
                                                                    {100, 0.24},//8
                                                                    {200, 0.20},//9
                                                                    {500, 0.18},//10
                                                                   };
   QMap <unsigned int, double>::iterator it_Coeff_Simul_2 = map_Coefficient_Of_Simultaneity_2.begin();

   //Коэффициенты одновременности для жилых домов с электроплитами
   QMap <unsigned int, double> map_Coefficient_Of_Simultaneity_3 = {
                                                                    {1, 1},     //1
                                                                    {2, 0.73},  //2
                                                                    {3, 0.62},  //3
                                                                    {5, 0.50},  //4
                                                                    {10, 0.38}, //5
                                                                    {20, 0.29}, //6
                                                                    {50, 0.22}, //7
                                                                    {100, 0.17},//8
                                                                    {200, 0.15},//9
                                                                    {500, 0.12},//10
                                                                   };
   QMap <unsigned int, double>::iterator it_Coeff_Simul_3 = map_Coefficient_Of_Simultaneity_3.begin();

   //Коэффициенты одновременности для производственных потребителей
   QMap <unsigned int, double> map_Coefficient_Of_Simultaneity_4 = {
                                                                    {1, 1},     //1
                                                                    {2, 0.85},  //2
                                                                    {3, 0.80},  //3
                                                                    {5, 0.75},  //4
                                                                    {10, 0.65}, //5
                                                                    {20, 0.55}, //6
                                                                    {50, 0.47}, //7
                                                                    {100, 0.40},//8
                                                                    {200, 0.35},//9
                                                                    {500, 0.30},//10
                                                                   };
   QMap <unsigned int, double>::iterator it_Coeff_Simul_4 = map_Coefficient_Of_Simultaneity_4.begin();

   QMap <double, double> map_Power_Supplements = {
                                                  {0, 0},       //0
                                                  {0.2,0.2},    //1
                                                  {0.4,0.3},    //2
                                                  {0.6,0.4},    //3
                                                  {0.8,0.5},    //4
                                                  {1.0,0.6},    //5
                                                  {2.0,1.2},    //6
                                                  {3.0,1.8},    //7
                                                  {4.0,2.4},    //8
                                                  {5.0,3.0},    //9
                                                  {6.0,3.6},    //10
                                                  {7.0,4.2},    //11
                                                  {8.0,4.8},    //12
                                                  {9.0,5.4},    //13
                                                  {10.0,6.0},   //14
                                                  {12.0,7.3},   //15
                                                  {14.0,8.5},   //16
                                                  {16.0,9.8},   //17
                                                  {18.0,11.2},  //18
                                                  {20.0,12.5},  //19
                                                  {22.0,13.8},  //20
                                                  {24.0,15.0},  //21
                                                  {26.0,16.4},  //22
                                                  {28.0,17.7},  //23
                                                  {30.0,19.0},  //24
                                                  {32.0,20.4},  //25
                                                  {35.0,22.8},  //26
                                                  {40.0,26.5},  //27
                                                  {45.0,30.2},  //28
                                                  {50.0,34.0},  //29
                                                  {55.0,37.5},  //30
                                                  {60.0,41.0},  //31
                                                  {65.0,44.5},  //32
                                                  {70.0,48.0},  //33
                                                  {80.0,55.0},  //34
                                                  {90.0,62.0},  //35
                                                  {100.0,69.0}, //36
                                                  {110.0,76.0}, //37
                                                  {120.0,84.0}, //38
                                                  {130.0,92.0}, //39
                                                  {140.0,100.0},//40
                                                  {150.0,108.0},//41
                                                  {160.0,116.0},//42
                                                  {170.0,123.0},//43
                                                  {180.0,130.0},//44
                                                  {190.0,140.0},//45
                                                  {200.0,150.0},//46
                                                  {210.0,158.0},//47
                                                  {220.0,166.0},//48
                                                  {230.0,174.0},//49
                                                  {240.0,182.0},//50
                                                  {250.0,190.0},//51
                                                  {260.0,198.0},//52
                                                  {270.0,206.0},//53
                                                  {280.0,214.0},//54
                                                  {290.0,222.0},//55
                                                  {300.0,230.0} //56
                                                 };
   QMap <double, double>::iterator it_Power_Supplements = map_Power_Supplements.begin();

   QVector <int> vec_Return_Index_For_ComboBox;//вектор для записи индексов ComboBox на участках

   ComboBoxModel* model;

   Ui::MainWindow *ui;

private slots:
    void on_lineEdit_editingFinished();//слот установливает число нагрузок и количество строк для ввода исходных даных
    void slot_Delete_Line_Window_1();//слот для обнуления строк в первом окне
    void slot_Delete_Line_Window_2();//Слот для обнуления строк во втором окне
    void slot_Delete_Line_Window_4();//слот для обнуления строк в окне перерасчета
    void pushButton_2_clicked();//слот для записи исходных данных в вектора
    void on_comboBox_currentIndexChanged(int index);//слот для установки типа нагрузки
    void slot_Calculation_Of_Loads();//слот для расчета нагрузок по участкам
    void slot_Cleaning_Graphics_Recalculation();//очистка графика перерасчета
    void slot_Set_Validator_For_Activ_Power();//слот для установки валидатора для окна ввода активных мощностей
};
/********************************************************************************************************************/
#endif // MAINWINDOW_H
