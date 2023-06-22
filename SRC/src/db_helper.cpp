#include "db_helper.h"
#include "ui_db_helper.h"
#include <QtSql>
#include <QPushButton>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>

bool table_ready = false;

bool db_open = false; // TODO: do a proper check
QSqlDatabase db;
void get_db_connection() {
//	if (!QSqlDatabase::contains("MyDBConnection"))
//		qDebug() << "ALERT: Connection already exists!!";
	if (db_open) return;
	db = QSqlDatabase::addDatabase("QODBC");
	QString dsn = QString("DRIVER={SQL SERVER};Server=LOCALHOST\\SQLEXPRESS;Database=master;Trusted_Connection=True;");
	db.setConnectOptions();
	db.setDatabaseName(dsn);
	db_open = true;
}

QString get_db_name() {
#ifndef MY_DB_NAME
	qDebug() << "Error: The variable 'MY_DB_NAME' is not defined.";
	return NULL;
#endif
	return QStringLiteral(MY_DB_NAME);
}

QString DB_NAME = get_db_name();

void add_to_db_table(QString table_name, QList<QString> list_columns, QList<QString> list_values) {
	get_db_connection();
    if (db.open()) {
        QSqlQuery qry;

        QString db_path = QString("[" + DB_NAME + "].[dbo].[%1]").arg(table_name);
        QList<QString> placeholders;
        for (int i = 0; i < list_columns.size(); i++) {
            placeholders.append(QString(":%1").arg(list_columns[i]));
            list_columns[i] = QString("[%1]").arg(list_columns[i]);
        }
        QString sQuery = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(db_path).arg(list_columns.join(",")).arg(placeholders.join(","));
        qry.prepare(sQuery);
        for (int i = 0; i < placeholders.size(); i++) {
            qry.bindValue(placeholders[i], list_values[i]);
        }
        if (qry.exec()) {
            qDebug() << "Record inserted.";
        } else {
            qDebug() << "Error - Could not insert record: " << db.lastError();
        }
    } else {
        qDebug() << "Error: Could not connect to db.";
	}
}

void set_table_headers(QTableWidget* table, QStringList headers = {}) {
	table->setColumnCount(headers.size());
	table->setHorizontalHeaderLabels(headers);
}

QPushButton* add_table_button(QTableWidget* table, int i, int j, QString text = "") {
	QWidget* pWidget = new QWidget();
	QPushButton* button = new QPushButton();
	button->setText(text);
	QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
	pLayout->addWidget(button);
	pLayout->setAlignment(Qt::AlignCenter);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pWidget->setLayout(pLayout);
	table->setCellWidget(i, j, pWidget);
	return button;
}

QList<QMap<QString, QString>> query_db(QString query_str) {
	get_db_connection();
	if (!db.open()) {
		qDebug() << "Error: Could not connect to db.";
	}

	QSqlQuery qry(query_str);
	QSqlRecord rec = qry.record();
	QList<QMap<QString, QString>> records_list;
	while (qry.next()) {
		QMap<QString, QString> record_map;
		for (int i = 0; i < rec.count(); i++) {
			QString value = (qry.isNull(i) ? NULL : qry.value(i).toString());
			record_map.insert(rec.fieldName(i), value);
		}
		records_list.append(record_map);
	}
	return records_list;
}

QList<QMap<QString, QString>> get_records(QString table_name) {
	QString db_path = QString("[" + DB_NAME + "].[dbo].[%1]").arg(table_name);
	QList<QMap<QString, QString>> records_list = query_db(QString("SELECT * FROM %1").arg(db_path));
	return records_list;
}

void update_record(QString table_name, int id, QString column, QString value) {
	QString db_path = QString("[" + DB_NAME + "].[dbo].[%1]").arg(table_name);
	query_db(QString("UPDATE %1 SET %2 = '%3' WHERE id = %4").arg(db_path).arg(column, value).arg(id));
}

void delete_record(QString table_name, int id) {
	QString db_path = QString("[" + DB_NAME + "].[dbo].[%1]").arg(table_name);
	query_db(QString("DELETE FROM %1 WHERE id = %4").arg(db_path).arg(id));
	qDebug() << QString("Deleted id %1 from table %2").arg(id).arg(table_name);
}

void create_table(QString table_name, QList<ColumnDict> columns) {
	get_db_connection();

    QString automatic_date_columns = "created_at datetime NOT NULL DEFAULT CURRENT_TIMESTAMP, updated_at datetime NOT NULL DEFAULT CURRENT_TIMESTAMP";

    if (db.open()) {
        QSqlQuery qry;
        QString columns_string = "";
        foreach (ColumnDict column, columns) {
            columns_string += QString(" %1 %2 %3,")
                .arg(column.column_name)
                .arg(column.column_type)
                .arg(column.can_be_null ? "NULL" : "NOT NULL");
        }
		if (columns_string.endsWith(",")) columns_string.chop(1);
		QString sQuery = QString("CREATE TABLE [%1].[dbo].[%2] (id int IDENTITY(1,1) PRIMARY KEY,%3,%4)").arg(DB_NAME).arg(table_name).arg(columns_string).arg(automatic_date_columns);
        qDebug() << sQuery;
        qry.prepare(sQuery);
        if (qry.exec()) {
            qDebug() << "Record inserted.";
        } else {
            qDebug() << "Error - Could not insert record: " << db.lastError();
        }
    } else {
        qDebug() << "Error: Could not connect to db.";
	}
}

void create_table_update_trigger(QString table_name) {
	get_db_connection();

    if (db.open()) {
        QSqlQuery qry;
		QString useQuery = QString("USE %1;").arg(DB_NAME);
		QString triggerQuery = QString("CREATE TRIGGER trg_%2_updated_at ON [%1].[dbo].[%2] AFTER UPDATE AS UPDATE [%1].[dbo].[%2] SET updated_at = CURRENT_TIMESTAMP WHERE id IN (SELECT id FROM inserted)").arg(DB_NAME).arg(table_name);

        if (qry.exec(useQuery) && qry.exec(triggerQuery)) {
            qDebug() << "Trigger created successfully.";
        } else {
            // TODO: log la query dans les logs d'erreurs et uniformiser tout ça
            qDebug() << "Error - Could not create trigger: " << db.lastError();
        }
    } else {
        qDebug() << "Error: Could not connect to db.";
	}
}

DB_helper::DB_helper(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DB_helper)
{
	ui->setupUi(this);

	display_table("meeting");
}

void DB_helper::display_table(QString table_name) {
	table_ready = false;
	current_table = table_name;
	QTableWidget* table = this->ui->tableWidget;
	table->setColumnCount(0);
	table->setRowCount(0);
	QList<QMap<QString,QString>> records = get_records(table_name);
	if (records.size() <= 0) {
		set_table_headers(ui->tableWidget, {"No data."});
		return;
	}
	QStringList headers;
	for (int i = 0; i < records[0].size(); i++) {
		headers.append(records[0].keys()[i]);
		if (records[0].keys()[i] == "enterprise_id") {
			qDebug() << records[0]["enterprise_id"];
		}
	}
	headers.append("Effacer");
	set_table_headers(ui->tableWidget, headers);
	for (int i = 0; i < records.size(); i++) {
		table->insertRow(i);
		for (int j = 0; j < records[i].size(); j++) {
			QString cell_text = records[i].values()[j];
			QTableWidgetItem* cell = table->item(i, j);
			if (!cell) {
				cell = new QTableWidgetItem;
				table->setItem(i, j, cell);
			}
			cell->setText(cell_text);
		}
		QPushButton* delete_button = add_table_button(table, i, records[0].size(), "Delete");
		connect(delete_button, &QPushButton::clicked, [=]{
			delete_record(current_table, records[i].value("id").toInt());
			display_table(current_table);
		});
	}
	table_ready = true;
}

DB_helper::~DB_helper()
{
	db.close();
	QSqlDatabase::removeDatabase("QODBC");
	delete ui;
}

void DB_helper::on_searchButton_clicked()
{
	display_table(ui->searchComboBox->currentText());
}


void DB_helper::on_tableWidget_cellChanged(int row, int column)
{
	if (!table_ready) return;
	QList<QMap<QString,QString>> records = get_records(current_table);
	if (records.size() > 0) {
		QString changed_col = records[row].keys()[column];
		if (changed_col == "id") return;
		QString new_value = ui->tableWidget->item(row, column)->text();
		if (new_value == "") new_value = NULL;
		update_record(current_table, records[row].value("id").toInt(), changed_col, new_value);
	}
}

