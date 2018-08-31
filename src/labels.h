#ifndef LABELS_H
#define LABELS_H

#include <QListWidgetItem>
#include <QJsonObject>
#include <QHash>
#include <QObject>

class LabelInfo  {
public:

	QString name         ;
	QString categorie    ;
	int     id           ;
	int     id_categorie ;
	QColor  color        ;
	QListWidgetItem *item;
	LabelInfo();
	LabelInfo(QString name, QString categorie, int id, int id_categorie, QColor color);
	void read(const QJsonObject &json);
	void write(QJsonObject &json) const;


};

inline bool operator==(const LabelInfo &l1, const LabelInfo &l2)
{
    return l1.name == l2.name && l1.categorie == l1.categorie && l1.id == l2.id;
}

inline uint qHash(const LabelInfo &label, uint seed = 0) {
    return qHash(label.name + label.categorie + QString::number(label.id), seed^0xa03f);
}

class Name2Labels : public QMap<QString, LabelInfo> {
public:
	void read(const QJsonObject &json);
	void write(QJsonObject &json) const;
};

class Id2Labels : public QMap<int,const LabelInfo*> { };

Id2Labels getId2Label(const Name2Labels& labels);
Name2Labels defaultLabels();

#endif
