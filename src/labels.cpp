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

    labels[QObject::tr("Unlabeled")           ] = LabelInfo(QObject::tr("Unlabeled"), "void"         ,  0, 0, QColor(  0,  0,  0));
    labels[QObject::tr("Debris")         ] = LabelInfo(QObject::tr("Debris"), "void"         ,  1, 1, QColor(58, 156, 147) );
    labels[QObject::tr("Sky")] = LabelInfo(QObject::tr("Sky"), "void",  2, 2, QColor(0, 177, 247) );
    labels[QObject::tr("Obstacle")          ] = LabelInfo(QObject::tr("Obstacle"), "void"         ,  3, 3, QColor(94, 30, 104) );
    labels[QObject::tr("Ground")              ] = LabelInfo(QObject::tr("Ground"), "void"         ,  4, 4, QColor(191, 119, 56) );
    labels[QObject::tr("Building")              ] = LabelInfo(QObject::tr("Building"), "void"         ,  5, 5, QColor(102, 102, 102) );
    labels[QObject::tr("Road")             ] = LabelInfo(QObject::tr("Road"), "void"         ,  6, 6, QColor(182, 179, 182));
    labels[QObject::tr("Pipe")              ] = LabelInfo(QObject::tr("Pipe"),              "void",  7, 7, QColor(243, 15, 190));
    labels[QObject::tr("Building Material") ] = LabelInfo(QObject::tr("Building Material"), "void",  8, 8, QColor(230, 225, 54));
    labels[QObject::tr("Big Vegetation")            ] = LabelInfo(QObject::tr("Big Vegetation"), "void",   9, 9, QColor(60, 112, 60));
    labels[QObject::tr("Small Vegetation")             ] = LabelInfo(QObject::tr("Small Vegetation"), "void",   10, 10, QColor(146, 243, 146));
    labels[QObject::tr("Manhole")          ] = LabelInfo(QObject::tr("Manhole"), "void"         ,  11, 11, QColor(122, 3, 51));
    labels[QObject::tr("Water")            ] = LabelInfo(QObject::tr("Water"), "void" ,  12, 12, QColor(164, 216, 255));
    labels[QObject::tr("Person")                ] = LabelInfo(QObject::tr("Person"), "void" ,  13, 13, QColor(250, 0, 55));
    labels[QObject::tr("Animal")               ] = LabelInfo(QObject::tr("Animal"), "void" ,  14, 14, QColor(178, 20, 50));
    labels[QObject::tr("Vehicle")          ] = LabelInfo(QObject::tr("Vehicle"), "void" ,  15, 15, QColor(0, 30, 130));

	//QVector<QColor> cmap = colorMap(labels.size());
	//qDebug() << "cmlap size = " << cmap.size();
	//QMapIterator<QString, LabelInfo> it(labels);
	//int i = 0;
	//while (it.hasNext()) {
	//	it.next();
	//	labels[it.key()].color = cmap[i++];
	//}
	//labels["unlabeled"].color = QColor(0, 0, 0);
	
	return labels;

}
