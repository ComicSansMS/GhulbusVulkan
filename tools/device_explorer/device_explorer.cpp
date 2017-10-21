
#include <QApplication>

#include <main_window.hpp>


int main(int argc, char* argv[])
{
    QApplication the_app(argc, argv);

    Ui::MainWindow main_window;
    main_window.setWindowTitle("gbVk Device Explorer");
    main_window.show();

    return the_app.exec();
}

