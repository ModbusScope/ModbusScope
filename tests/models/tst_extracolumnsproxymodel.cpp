#include "tst_extracolumnsproxymodel.h"

#include "models/extracolumnsproxymodel.h"

#include <QAbstractItemModelTester>
#include <QMimeData>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTest>

#include <memory>

namespace {

//! Two extra columns that show which extra column and row a cell is.
class TwoExtraColumns : public ExtraColumnsProxyModel
{
public:
    explicit TwoExtraColumns(QObject* parent = nullptr) : ExtraColumnsProxyModel(2, parent)
    {
    }

    using ExtraColumnsProxyModel::extraColumnsChanged;

protected:
    QVariant extraColumnData(int row, int extraColumn, int role) const override
    {
        if (role != Qt::DisplayRole)
        {
            return QVariant();
        }
        return QStringLiteral("extra%1 row%2").arg(extraColumn).arg(row);
    }

    QVariant extraHeaderData(int extraColumn, int role) const override
    {
        return role == Qt::DisplayRole ? QVariant(QStringLiteral("Extra %1").arg(extraColumn)) : QVariant();
    }
};

} // namespace

void TestExtraColumnsProxyModel::init()
{
    _pSource = new QStandardItemModel(3, 2, this);
    const QStringList names = { QStringLiteral("c"), QStringLiteral("a"), QStringLiteral("b") };
    for (int row = 0; row < names.size(); row++)
    {
        _pSource->setItem(row, 0, new QStandardItem(names.at(row)));
        _pSource->setItem(row, 1, new QStandardItem(QString::number(row)));
    }

    _pProxy = new TwoExtraColumns(this);
    _pProxy->setSourceModel(_pSource);
}

void TestExtraColumnsProxyModel::cleanup()
{
    delete _pProxy;
    _pProxy = nullptr;
    delete _pSource;
    _pSource = nullptr;
}

void TestExtraColumnsProxyModel::modelTester()
{
    QAbstractItemModelTester tester(_pProxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

    _pSource->appendRow({ new QStandardItem(QStringLiteral("d")), new QStandardItem(QStringLiteral("3")) });
    _pSource->removeRow(1);
    _pSource->insertColumn(1);
    _pSource->sort(0);
    _pSource->removeColumn(1);
    _pSource->clear();
}

void TestExtraColumnsProxyModel::extraColumnsAfterSourceColumns()
{
    QCOMPARE(_pProxy->columnCount(), 4);
    QCOMPARE(_pProxy->firstExtraColumn(), 2);
    QCOMPARE(_pProxy->extraColumnIndex(1), -1);
    QCOMPARE(_pProxy->extraColumnIndex(2), 0);
    QCOMPARE(_pProxy->extraColumnIndex(3), 1);
    QCOMPARE(_pProxy->extraColumnIndex(4), -1);

    QCOMPARE(_pProxy->index(0, 0).data().toString(), QStringLiteral("c"));
    QCOMPARE(_pProxy->index(1, 3).data().toString(), QStringLiteral("extra1 row1"));
    QCOMPARE(_pProxy->headerData(2, Qt::Horizontal).toString(), QStringLiteral("Extra 0"));
}

void TestExtraColumnsProxyModel::extraColumnsReadOnlyWithoutSource()
{
    const QModelIndex extraIndex = _pProxy->index(0, 2);

    QVERIFY(!_pProxy->mapToSource(extraIndex).isValid());
    QVERIFY(!_pProxy->setData(extraIndex, QStringLiteral("x")));
    QVERIFY(!_pProxy->flags(extraIndex).testFlag(Qt::ItemIsEditable));
    QVERIFY(_pProxy->flags(extraIndex).testFlag(Qt::ItemIsSelectable));
}

void TestExtraColumnsProxyModel::extraColumnsHaveNoChildren()
{
    const QModelIndex extraIndex = _pProxy->index(0, 2);

    QCOMPARE(_pProxy->rowCount(extraIndex), 0);
    QVERIFY(!_pProxy->hasChildren(extraIndex));
    QVERIFY(!_pProxy->index(0, 2, _pProxy->index(0, 0)).isValid());
}

void TestExtraColumnsProxyModel::rowSelectionMapsToSourceColumnsOnly()
{
    const QItemSelection selection(_pProxy->index(1, 0), _pProxy->index(1, 3));

    const QItemSelection sourceSelection = _pProxy->mapSelectionToSource(selection);

    QCOMPARE(sourceSelection.size(), 1);
    QCOMPARE(sourceSelection.first().left(), 0);
    QCOMPARE(sourceSelection.first().right(), 1);
    QCOMPARE(sourceSelection.first().top(), 1);
}

void TestExtraColumnsProxyModel::extraColumnsFollowSourceColumnInsert()
{
    const QPersistentModelIndex extraIndex = _pProxy->index(0, 2);

    _pSource->insertColumn(1);

    QCOMPARE(_pProxy->firstExtraColumn(), 3);
    QCOMPARE(extraIndex.column(), 3);
    QCOMPARE(extraIndex.data().toString(), QStringLiteral("extra0 row0"));
}

//! Sorting the source moves a persistent index in an extra column with its row.
void TestExtraColumnsProxyModel::layoutChangeKeepsExtraColumnIndexOnItsRow()
{
    const QPersistentModelIndex extraIndex = _pProxy->index(0, 3);
    const QPersistentModelIndex sourceColumnIndex = _pProxy->index(0, 0);
    QSignalSpy spyLayoutChanged(_pProxy, &QAbstractItemModel::layoutChanged);

    _pSource->sort(0); /* "c" moves from row 0 to row 2 */

    QCOMPARE(spyLayoutChanged.count(), 1);
    QCOMPARE(sourceColumnIndex.row(), 2);
    QCOMPARE(extraIndex.row(), 2);
    QCOMPARE(extraIndex.column(), 3);
}

void TestExtraColumnsProxyModel::extraColumnsChangedEmitsForExtraColumnsOnly()
{
    TwoExtraColumns proxy;
    proxy.setSourceModel(_pSource);
    QSignalSpy spyDataChanged(&proxy, &QAbstractItemModel::dataChanged);

    proxy.extraColumnsChanged(0, 1);

    QCOMPARE(spyDataChanged.count(), 1);
    QCOMPARE(spyDataChanged.at(0).at(0).toModelIndex(), proxy.index(0, 2));
    QCOMPARE(spyDataChanged.at(0).at(1).toModelIndex(), proxy.index(1, 3));
}

//! A drop on an extra column inserts the row in the source without adding source columns.
void TestExtraColumnsProxyModel::dropOnExtraColumnKeepsSourceColumns()
{
    std::unique_ptr<QMimeData> pMimeData(_pProxy->mimeData({ _pProxy->index(1, 0) }));
    QVERIFY(pMimeData != nullptr);

    QVERIFY(_pProxy->canDropMimeData(pMimeData.get(), Qt::CopyAction, 0, 2, QModelIndex()));
    QVERIFY(_pProxy->dropMimeData(pMimeData.get(), Qt::CopyAction, 0, 2, QModelIndex()));

    QCOMPARE(_pSource->rowCount(), 4);
    QCOMPARE(_pSource->columnCount(), 2);
    QCOMPARE(_pSource->item(0, 0)->text(), QStringLiteral("a"));
}

QTEST_GUILESS_MAIN(TestExtraColumnsProxyModel)
