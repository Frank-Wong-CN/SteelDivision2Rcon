#include "view_models.h"

BlacklistModel::BlacklistModel(QObject *parent):
    QAbstractTableModel{parent} {}

const QVector<QString> ClientInfoItemData::s_Strings = {
    "",
    "Very Easy",
    "Easy",
    "Medium",
    "Hard",
    "Very Hard",
    "Hardest"
};

QString ClientInfoItemData::DifficultyString() const {
    if (m_Difficulty > 6 || m_Difficulty < 0)
        return s_Strings[0];
    return s_Strings[m_Difficulty];
}

ClientInfoDifficultyDropdownDelegate::ClientInfoDifficultyDropdownDelegate(QObject *parent):
    QStyledItemDelegate{parent} {}

QSize ClientInfoDifficultyDropdownDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.rect.width(), option.fontMetrics.height() + 4);
}

QWidget *ClientInfoDifficultyDropdownDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QComboBox *comboBox = new QComboBox(parent);
    int i = 0;

    for (const QString &it : ClientInfoItemData::s_Strings) {
        comboBox->addItem(it, i);
        comboBox->setItemData(i++, { Qt::AlignCenter | Qt::AlignVCenter }, Qt::TextAlignmentRole);
    }

    return comboBox;
}

void ClientInfoDifficultyDropdownDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    uint32_t value = index.model()->data(index, Qt::DisplayRole).toUInt();
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    comboBox->setCurrentIndex(value);
}

void ClientInfoDifficultyDropdownDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    uint32_t value = comboBox->currentData().toUInt();
    model->setData(index, value, Qt::EditRole);
}

void ClientInfoDifficultyDropdownDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    editor->setGeometry(option.rect);
}

bool ClientInfoDifficultyDropdownDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        ClientInfoModel *parent = qobject_cast<ClientInfoModel *>(this->parent());
        ClientList *list = parent->m_pClientList;
        int row = index.row();
        // Only applicable to computer clients
        if ((*list)[row].IsComputer()) {
            QTableView *view = parent->m_pView;
            if (QWidget *editor = createEditor(qobject_cast<QWidget *>(view), option, index)) {
                setEditorData(editor, index);
                QRect targetRect = option.rect;
                targetRect.setX(targetRect.x() + 6);
                targetRect.setY(targetRect.y() + 4);
                targetRect.setWidth(targetRect.width() * 1.03);
                editor->setGeometry(targetRect);
                editor->setFocus();
                static_cast<QComboBox*>(editor)->showPopup();

                // Connect to handle immediate value change inside the editorEvent
                QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
                if (comboBox) {
                    connect(comboBox, &QComboBox::currentIndexChanged, this, [this, view, comboBox, model, index]() mutable {
                        setModelData(comboBox, model, index);
                    });
                }
                return true;
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

ClientInfoSetDeckDelegate::ClientInfoSetDeckDelegate(QObject *parent):
    QAbstractItemDelegate{parent} {}

void ClientInfoSetDeckDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QPushButton button;
    button.setText(QObject::tr("Set Deck"));
    button.resize(sizeHint(option, index));
    painter->save();
    painter->translate(option.rect.topLeft());
    button.render(painter);
    painter->restore();
}

QSize ClientInfoSetDeckDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.rect.width(), option.rect.height());
}

bool ClientInfoSetDeckDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (option.rect.contains(mouseEvent->pos())) {
            emit buttonClicked(index.row());
            return true;
        }
    }
    return false;
}

ClientInfoModel::ClientInfoModel(QObject *parent, QTableView *tableView, ClientList *model):
    QAbstractTableModel{parent},
    m_pClientList(model),
    m_pView(tableView),
    m_pDropdownDelegate(new ClientInfoDifficultyDropdownDelegate(this)),
    m_pSetDeckDelegate(new ClientInfoSetDeckDelegate(this)) {
    QObject::connect(m_pSetDeckDelegate, &ClientInfoSetDeckDelegate::buttonClicked, this, [this](uint32_t row) {
        if (row > m_pClientList->size() || row < 0) return;
        emit setDeck((*m_pClientList)[row].m_ClientID);
    });
    QObject::connect(this, &ClientInfoModel::dataChanged, this, [=, this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> roles) {
        if (!topLeft.isValid() || topLeft.column() != 2) return;
        int row = topLeft.row();
        if ((*m_pClientList)[row].m_Difficulty == 0) return;
        emit setDifficulty((*m_pClientList)[row].m_ClientID, (*m_pClientList)[row].m_Difficulty);
    });
    m_pView->setModel(this);
    m_pView->setItemDelegateForColumn(2, m_pDropdownDelegate);
    m_pView->setItemDelegateForColumn(3, m_pSetDeckDelegate);
}

int ClientInfoModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_pClientList->size();
}

int ClientInfoModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 4; // ID, Nickname, Difficulty, Set Deck
}

QVariant ClientInfoModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_pClientList->size() || index.column() >= 4)
        return QVariant();

    const ClientInfoItemData &item = (*m_pClientList)[index.row()];

    switch (role) {
    case Qt::TextAlignmentRole:
        return { Qt::AlignCenter | Qt::AlignVCenter };
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (index.column()) {
        case 0: return item.m_ClientID;
        case 1: return item.m_ClientNickname;
        case 2: return item.DifficultyString();
        default: return QVariant();
        }
        break;
    default:
        break;
    }
    return QVariant();
}

bool ClientInfoModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_pClientList->size() || index.column() >= 4)
        return false;

    ClientInfoItemData &item = (*m_pClientList)[index.row()];

    switch (role) {
    case Qt::EditRole:
        switch (index.column()) {
        case 0: item.m_ClientID = value.toULongLong(); break;
        case 1: item.m_ClientNickname = value.toString(); break;
        case 2: item.m_Difficulty = value.toUInt(); break;
        case 3: break;
        default: return false;
        }
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, {role});
    return true;
}

QVariant ClientInfoModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return "ID";
        case 1: return QObject::tr("Nickname");
        case 2: return QObject::tr("Difficulty");
        case 3: return "";
        default: return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags ClientInfoModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    // if (index.column() == 2)
    //     flags |= Qt::ItemIsEditable;
    return flags;
}

bool ClientInfoModel::clientExists(uint64_t id) {
    for (auto &cl : *m_pClientList) {
        if (cl.m_ClientID == id) return true;
    }
    return false;
}

void ClientInfoModel::addClient(const QMap<uint64_t, QString> *rawClientList) {
    int rowCount = m_pClientList->size();
    erase_if(*m_pClientList, [=, this](const ClientInfoItemData &cl) {
        return !rawClientList->contains(cl.m_ClientID);
    });

    QList<uint64_t> keys = rawClientList->keys();
    for (auto &it : keys) {
        if (!clientExists(it)) {
            m_pClientList->append({ it, (*rawClientList)[it], 0 });
        }
    }

    int newCount = m_pClientList->size();

    if (rowCount > newCount) {
        beginRemoveRows(QModelIndex(), newCount, rowCount - 1);
        for (int row = rowCount - 1; row >= newCount; --row) {
            removeRow(row);
        }
        endRemoveRows();
    }
    else if (rowCount < newCount) {
        beginInsertRows(QModelIndex(), rowCount, newCount - 1);
        endInsertRows();
    }

    updateView();
}

void ClientInfoModel::updateView() {
    emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, columnCount() - 1), {Qt::EditRole});
}

