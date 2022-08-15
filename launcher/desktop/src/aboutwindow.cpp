#include "aboutwindow.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QVBoxLayout>

#include "config.h"
#include "license.h"

AboutWindow::AboutWindow(QWidget* widget) : QDialog(widget) {
    setWindowTitle("About");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    auto mainLabel = new QLabel();
    mainLabel->setText(QString("<h2>Astra</h2>\nVersion %1").arg(version));
    mainLayout->addWidget(mainLabel);

    auto aboutWidget = new QWidget();
    auto aboutLayout = new QVBoxLayout();
    aboutWidget->setLayout(aboutLayout);

    auto aboutLabel = new QLabel();
    aboutLabel->setText("Cross-platform FFXIV launcher");
    aboutLayout->addWidget(aboutLabel);

    auto websiteLabel = new QLabel();
    websiteLabel->setText("<a href='https://xiv.zone/astra'>https://xiv.zone/astra</a>");
    websiteLabel->setOpenExternalLinks(true);
    aboutLayout->addWidget(websiteLabel);

    auto licenseLabel = new QLabel();
    licenseLabel->setText("<a href='a'>License: GNU General Public License Version 3</a>");
    connect(licenseLabel, &QLabel::linkActivated, [this] {
        QDialog* licenseDialog = new QDialog(this);
        licenseDialog->setWindowTitle("License Agreement");

        QVBoxLayout* layout = new QVBoxLayout();
        licenseDialog->setLayout(layout);

        QPlainTextEdit* licenseEdit = new QPlainTextEdit();
        licenseEdit->setPlainText(license);
        licenseEdit->setReadOnly(true);
        layout->addWidget(licenseEdit);

        licenseDialog->show();
    });
    aboutLayout->addWidget(licenseLabel);

    aboutLayout->addStretch();

    auto authorsWidget = new QWidget();
    auto authorsLayout = new QVBoxLayout();
    authorsWidget->setLayout(authorsLayout);

    auto authorNameLabel = new QLabel();
    authorNameLabel->setText("Joshua Goins");

    QFont boldFont = authorNameLabel->font();
    boldFont.setBold(true);
    authorNameLabel->setFont(boldFont);

    authorsLayout->addWidget(authorNameLabel);

    auto authorRoleLabel = new QLabel();
    authorRoleLabel->setText("Maintainer");
    authorsLayout->addWidget(authorRoleLabel);

    authorsLayout->addStretch();

    auto tabWidget = new QTabWidget();
    tabWidget->addTab(aboutWidget, "About");
    tabWidget->addTab(authorsWidget, "Authors");
    mainLayout->addWidget(tabWidget);
}