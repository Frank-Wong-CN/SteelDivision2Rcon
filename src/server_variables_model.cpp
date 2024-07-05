#include "server_variables_model.h"

// ServerVariableSelectedCheckboxDelegate implementation

ServerVariableSelectedCheckboxDelegate::ServerVariableSelectedCheckboxDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void ServerVariableSelectedCheckboxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
    QString checkboxText = checked ? "✅" : "⬜";
    QRect textRect = option.rect;
    painter->save();
    painter->drawText(textRect, Qt::AlignCenter, checkboxText);
    painter->restore();
}

QSize ServerVariableSelectedCheckboxDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QApplication::style()->sizeFromContents(QStyle::CT_CheckBox, &option, QSize());
}

QWidget *ServerVariableSelectedCheckboxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(parent)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return nullptr;
}

void ServerVariableSelectedCheckboxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    Q_UNUSED(editor)
}

void ServerVariableSelectedCheckboxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    Q_UNUSED(editor)
}

void ServerVariableSelectedCheckboxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    editor->setGeometry(option.rect);
}

bool ServerVariableSelectedCheckboxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::LeftButton) {
            bool value = model->data(index, Qt::DisplayRole).toBool();
            model->setData(index, !value, Qt::EditRole);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// ServerVariableValueOptionDropdownDelegate implementation

ServerVariableValueOptionDropdownDelegate::ServerVariableValueOptionDropdownDelegate(QObject *parent)
    : QAbstractItemDelegate(parent) {}

void ServerVariableValueOptionDropdownDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QVariant value = index.model()->data(index, Qt::DisplayRole);
    if (isValueOption(value)) {
        ValueOption valueOption = value.value<ValueOption>();
        QString displayText = QString("%1").arg(valueOption.m_PossibleValues.key(valueOption.m_Value));
        QApplication::style()->drawItemText(painter, option.rect, Qt::AlignCenter, QApplication::palette(), true, displayText);
    } else if (value.typeId() == QMetaType::QString) {
        QApplication::style()->drawItemText(painter, option.rect, Qt::AlignCenter | Qt::AlignVCenter, QApplication::palette(), true, value.toString());
    } else if (value.typeId() == QMetaType::Int) {
        QApplication::style()->drawItemText(painter, option.rect, Qt::AlignCenter | Qt::AlignVCenter, QApplication::palette(), true, value.toString());
    }
}

QSize ServerVariableValueOptionDropdownDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.rect.width(), option.fontMetrics.height() + 4);
}

QWidget *ServerVariableValueOptionDropdownDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QVariant value = index.model()->data(index, Qt::DisplayRole);

    if (isValueOption(value)) {
        QComboBox *comboBox = new QComboBox(parent);
        int i = 0;

        ValueOption valueOption = value.value<ValueOption>();
        QList<QString> keys = valueOption.m_PossibleValues.keys();
        if (valueOption.m_Value.typeId() == QMetaType::Int) {
            QMap<int, QString> sorting;
            for (const QString &key : keys) sorting.insert(valueOption.m_PossibleValues[key].toInt(), key);
            keys.clear();
            for (const auto &it: sorting) keys.push_back(it);
        }
        for (const QString &possibleValue : keys) {
            comboBox->addItem(possibleValue, valueOption.m_PossibleValues[possibleValue]);
            comboBox->setItemData(i++, { Qt::AlignCenter | Qt::AlignVCenter }, Qt::TextAlignmentRole);
        }
        return comboBox;
    } else if (value.typeId() == QMetaType::QString) {
        QLineEdit *lineEdit = new QLineEdit(parent);
        return lineEdit;
    } else if (value.typeId() == QMetaType::Int) {
        QSpinBox *spinBox = new QSpinBox(parent);
        spinBox->setRange(INT_MIN, INT_MAX);
        return spinBox;
    }
    return QAbstractItemDelegate::createEditor(parent, option, index);
}

void ServerVariableValueOptionDropdownDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QVariant value = index.model()->data(index, Qt::DisplayRole);

    if (QComboBox *comboBox = qobject_cast<QComboBox*>(editor)) {
        if (isValueOption(value)) {
            ValueOption valueOption = value.value<ValueOption>();
            QVariant currentValue = valueOption.m_Value;

            int idx = comboBox->findData(currentValue);
            if (idx != -1) {
                comboBox->setCurrentIndex(idx);
            }
        }
    } else if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
        if (value.typeId() == QMetaType::QString) {
            lineEdit->setText(value.toString());
        }
    } else if (QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor)) {
        if (value.typeId() == QMetaType::Int) {
            spinBox->setValue(value.toInt());
        }
    }
}


void ServerVariableValueOptionDropdownDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    if (QComboBox *comboBox = qobject_cast<QComboBox*>(editor)) {
        QVariant newValue = comboBox->currentData();
        if (isValueOption(model->data(index, Qt::DisplayRole))) {
            ValueOption valueOption = model->data(index, Qt::DisplayRole).value<ValueOption>();
            valueOption.m_Value = newValue;
            model->setData(index, QVariant::fromValue(valueOption), Qt::EditRole);
        }
    } else if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
        QString text = lineEdit->text();
        if (model->data(index, Qt::DisplayRole).typeId() == QMetaType::QString) {
            model->setData(index, text, Qt::EditRole);
        }
    } else if (QSpinBox *spinBox = qobject_cast<QSpinBox*>(editor)) {
        int value = spinBox->value();
        if (model->data(index, Qt::DisplayRole).typeId() == QMetaType::Int) {
            model->setData(index, value, Qt::EditRole);
        }
    }
}


void ServerVariableValueOptionDropdownDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    editor->setGeometry(option.rect);
}

bool ServerVariableValueOptionDropdownDelegate::isValueOption(const QVariant &value) const {
    return value.typeId() == qMetaTypeId<ValueOption>();
}

// ServerVariablesModel implementation

ServerVariablesModel::ServerVariablesModel(QObject *parent, QTableView *tableView, ServerVariables *model):
    QAbstractTableModel(parent),
    m_pServerVariables(model),
    m_pChkboxDelegate(new ServerVariableSelectedCheckboxDelegate(this)),
    m_pDropdownDelegate(new ServerVariableValueOptionDropdownDelegate(this)) {

    qRegisterMetaType<ValueOption>("ValueOption");

    tableView->setModel(this);
    tableView->setItemDelegateForColumn(0, m_pChkboxDelegate);
    tableView->setItemDelegateForColumn(2, m_pDropdownDelegate);
}

int ServerVariablesModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_pServerVariables->size();
}

int ServerVariablesModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 3; // Selected, Varname, Value
}

QVariant ServerVariablesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_pServerVariables->size() || index.column() >= 3)
        return QVariant();

    const ServerVariableItemData &item = (*m_pServerVariables)[index.row()];

    switch (role) {
    case Qt::TextAlignmentRole:
        return { Qt::AlignCenter | Qt::AlignVCenter };
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (index.column()) {
        case 0: return item.m_Selected ? true : false;
        case 1: return item.m_Varname;
        case 2: return item.m_Value;
        default: return QVariant();
        }
        break;
    default:
        break;
    }
    return QVariant();
}

bool ServerVariablesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_pServerVariables->size() || index.column() >= 3)
        return false;

    ServerVariableItemData &item = (*m_pServerVariables)[index.row()];

    switch (role) {
    case Qt::EditRole:
        switch (index.column()) {
        case 0: item.m_Selected = value.toBool(); break;
        case 1: item.m_Varname = value.toString(); break;
        case 2: item.m_Value = value; break;
        default: return false;
        }
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, {role});
    return true;
}

QVariant ServerVariablesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QObject::tr("Selected");
        case 1: return QObject::tr("Variable");
        case 2: return QObject::tr("Value");
        default: return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags ServerVariablesModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == 0)
        flags |= Qt::ItemIsEditable;
    if (index.column() == 2)
        flags |= Qt::ItemIsEditable;
    return flags;
}

ServerVariableItemData &ServerVariablesModel::getServerVariableAtIndex(const QModelIndex index) {
    return (*m_pServerVariables)[index.row()];
}

ServerVariableItemData &ServerVariablesModel::getServerVariable(const QString varname) {
    for (auto &it : *m_pServerVariables)
        if (it.m_Varname == varname) return it;
    return __Dummy;
}

void ServerVariablesModel::updateView(int column) {
    emit dataChanged(createIndex(0, column), createIndex(rowCount() - 1, column), {Qt::EditRole});
}

