#include "stringlisteditdialog.h"
#include <QStringListModel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include "stringlistedit.h"

StringListEditDialog::StringListEditDialog(
    const QStringList& strings, const QString& title, QWidget* parent)
  : QDialog(parent),
    m_model(new QStringListModel(strings, this)),
    m_formatEdit(new StringListEdit(m_model, this))
{
  setWindowTitle(title);
  auto vlayout = new QVBoxLayout(this);
  vlayout->addWidget(m_formatEdit);
  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                     QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  vlayout->addWidget(buttonBox);
}

QStringList StringListEditDialog::stringList() const {
  return m_model->stringList();
}
