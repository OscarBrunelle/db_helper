#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QtSql>

struct ColumnDict {
    QString column_name;
    QString column_type;
    bool can_be_null;
};

void get_db_connection();
void add_to_db_table(QString table_name, QList<QString> list_columns, QList<QString> list_values);
void update_record(QString table_name, int id, QString column, QString value);
void delete_record(QString table_name, int id);
QList<QMap<QString, QString>> get_records(QString table_name);
QSqlRecord query_db(QString table_name, QString query_str);
void create_table(QString table_name, QList<ColumnDict> columns);
void create_table_update_trigger(QString table_name);

namespace Ui {
class DB_helper;
}

class DB_helper : public QDialog
{
	Q_OBJECT

public:
	explicit DB_helper(QWidget *parent = nullptr);
	~DB_helper();

private slots:
	void on_searchButton_clicked();

	void on_tableWidget_cellChanged(int row, int column);

private:
	Ui::DB_helper *ui;
	QString current_table = "";
	void display_table(QString table_name);
};

#endif // DB_MANAGER_H
