#ifndef SERVER_VARIABLES_MODEL_H
#define SERVER_VARIABLES_MODEL_H

#include <QObject>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTableView>
#include <QAbstractTableModel>
#include <QAbstractItemDelegate>
#include <QStyleOptionButton>
#include <QStyledItemDelegate>

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

#include <QString>
#include <QVariant>
#include <QDateTime>
#include <QList>

struct ValueOption {
    QVariant m_Value;
    QMap<QString, QVariant> m_PossibleValues;

    ValueOption(): m_Value("err"), m_PossibleValues({}) {}
    ValueOption(QVariant val,  QMap<QString, QVariant> opt): m_Value(val), m_PossibleValues(opt) {}
};

Q_DECLARE_METATYPE(ValueOption)

struct ServerVariableItemData {
    bool m_Selected;
    QString m_Varname;
    QVariant m_Value; // Integer or String

    template <typename T>
    void SetValue(const T &value) {
        if (m_Value.typeId() == qMetaTypeId<ValueOption>())
            ((ValueOption *)m_Value.data())->m_Value = value;
        else m_Value = value;
    }

    template <typename T>
    const T &GetValue() const {
        if (m_Value.typeId() == qMetaTypeId<ValueOption>())
            return *(T *)(((ValueOption *)m_Value.data())->m_Value.data());
        else return *((T *)m_Value.data());
    }

    const QVariant GetVariantValue() const {
        if (m_Value.typeId() == qMetaTypeId<ValueOption>())
            return m_Value.value<ValueOption>().m_Value;
        else return m_Value;
    }

    int GetVarTypeId() {
        if (m_Value.typeId() == qMetaTypeId<ValueOption>())
            return m_Value.value<ValueOption>().m_Value.typeId();
        else return m_Value.typeId();
    }
};

typedef QList<ServerVariableItemData> ServerVariables;

class ServerVariableSelectedCheckboxDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ServerVariableSelectedCheckboxDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

class ServerVariableValueOptionDropdownDelegate : public QAbstractItemDelegate {
    Q_OBJECT
public:
    explicit ServerVariableValueOptionDropdownDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    bool isValueOption(const QVariant &value) const;
};

class ServerVariablesModel : public QAbstractTableModel {
    Q_OBJECT

    ServerVariableSelectedCheckboxDelegate *m_pChkboxDelegate;
    ServerVariableValueOptionDropdownDelegate *m_pDropdownDelegate;

    ServerVariableItemData __Dummy;
public:
    explicit ServerVariablesModel(QObject *parent, QTableView *tableView, ServerVariables *model);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    ServerVariableItemData &getServerVariableAtIndex(const QModelIndex index);
    ServerVariableItemData &getServerVariable(const QString varname);
    void updateView(int column);

private:
    ServerVariables *m_pServerVariables;
};

#endif // SERVER_VARIABLES_MODEL_H
