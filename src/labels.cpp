#include "labels.h"
#include "utils.h"

#include <QListWidgetItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>
#include <QColormap>
#include <QDebug>

LabelInfo::LabelInfo() {
	this->name = "unlabeled";
	this->categorie = "void";
	this->id = 0;
	this->id_categorie = 0;
	this->color = QColor(0, 0, 0);
	item = NULL;
}

LabelInfo::LabelInfo(QString name, QString categorie, int id, int id_categorie, QColor color) {
	this->name = name;
	this->categorie = categorie;
	this->id = id;
	this->id_categorie = id_categorie;
	this->color = color;
	item = NULL;
}

void LabelInfo::read(const QJsonObject &json) {
	id = json["id"].toInt();
	name = json["name"].toString();
	categorie = json["categorie"].toString();
	id_categorie = json["id_categorie"].toInt();
	QJsonArray jarray = json["color"].toArray();
	int r = jarray[0].toInt();
	int g = jarray[1].toInt();
	int b = jarray[2].toInt();
	color = QColor(r, g, b);
}

void LabelInfo::write(QJsonObject &json) const {
	json["id"] = id;
	json["name"] = name;
	json["categorie"] = categorie;
	json["id_categorie"] = id_categorie;
	QJsonArray jarray;
	jarray.append(color.red());
	jarray.append(color.green());
	jarray.append(color.blue());
	json["color"] = jarray;
}


void Name2Labels::read(const QJsonObject &json) {
	QJsonObject array = json["labels"].toObject();
	for (QJsonObject::iterator it = array.begin(); it != array.end(); it++) {
		QJsonObject object = it->toObject();
		LabelInfo label;
		label.read(object);
		(*this)[label.name] = label;
	}
}
void Name2Labels::write(QJsonObject &json) const {
	QMapIterator<QString, LabelInfo> it(*this);
	QJsonObject object;
	while (it.hasNext()) {
		it.next();
		const LabelInfo & label = it.value();
		QJsonObject child_object;
		label.write(child_object);
		object[it.key()] = child_object;
	}
	json["labels"] = object;
}


Id2Labels getId2Label(const Name2Labels& labels) {
	Id2Labels id_labels;
	QMapIterator<QString, LabelInfo> i(labels);
	while (i.hasNext()) {
		i.next();
		id_labels[i.value().id] = &i.value();
	}
	return id_labels;
}

Name2Labels defaultLabels() {
	Name2Labels labels;

    labels[QObject::tr("Неразмечаемый")           ] = LabelInfo(QObject::tr("Неразмечаемый"), "void"         ,  0, 0, QColor(  0,  0,  0));
        labels[QObject::tr("Небо")] = LabelInfo(QObject::tr("Небо"), "void",  1, 1, QColor(0, 177, 247) );
        labels[QObject::tr("Песок")          ] = LabelInfo(QObject::tr("Песок"), "void"         ,  2, 2, QColor(94, 30, 104) );
        labels[QObject::tr("Земля")              ] = LabelInfo(QObject::tr("Земля"), "void"         ,  3, 3, QColor(191, 119, 56) );
        labels[QObject::tr("Постройка")              ] = LabelInfo(QObject::tr("Постройка"), "void"         ,  4, 4, QColor(102, 102, 102) );
        labels[QObject::tr("Экскременты")             ] = LabelInfo(QObject::tr("Экскременты"), "void"         ,  5, 5, QColor(182, 179, 182));
        labels[QObject::tr("Мячик")              ] = LabelInfo(QObject::tr("Мячик"),              "void",  6, 6, QColor(243, 15, 190));
        labels[QObject::tr("Камень или скала") ] = LabelInfo(QObject::tr("Камень или скала"), "void",  7, 7, QColor(230, 225, 54));
        labels[QObject::tr("Деревья и кусты")            ] = LabelInfo(QObject::tr("Деревья и кусты"), "void",   8, 8, QColor(40, 140, 40));
        labels[QObject::tr("Подготовленная трава")             ] = LabelInfo(QObject::tr("Подготовленная трава"), "void",  9, 9, QColor(146, 243, 146));
        labels[QObject::tr("Дикая трава")             ] = LabelInfo(QObject::tr("Дикая трава"), "void",   10, 10, QColor(10, 250, 30));
        labels[QObject::tr("Лунка")          ] = LabelInfo(QObject::tr("Лунка"), "void"         ,  11, 11, QColor(122, 3, 51));
        labels[QObject::tr("Вода")            ] = LabelInfo(QObject::tr("Вода"), "void" ,  12, 12, QColor(164, 216, 255));
        labels[QObject::tr("Человек")                ] = LabelInfo(QObject::tr("Человек"), "void" ,  13, 13, QColor(250, 0, 55));
        labels[QObject::tr("Животное")               ] = LabelInfo(QObject::tr("Животное"), "void" ,  14, 14, QColor(178, 20, 50));
        labels[QObject::tr("Техника")          ] = LabelInfo(QObject::tr("Техника"), "void" ,  15, 15, QColor(0, 30, 130));
	
	return labels;

}
