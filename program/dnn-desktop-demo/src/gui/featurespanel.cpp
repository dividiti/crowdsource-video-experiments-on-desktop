#include "appconfig.h"
#include "appmodels.h"
#include "experimentcontext.h"
#include "featurespanel.h"
#include "utils.h"
#include "../ori/OriWidgets.h"

#include <QBoxLayout>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QRadioButton>
#include <QDialogButtonBox>

FeaturesPanel::FeaturesPanel(ExperimentContext* context, QWidget *parent) : QFrame(parent)
{
    setObjectName("featuresPanel");

    _context = context;
    connect(_context, SIGNAL(experimentStarted()), this, SLOT(experimentStarted()));
    connect(_context, SIGNAL(experimentFinished()), this, SLOT(experimentFinished()));

    _infoEngine = makeInfoLabel();
    _infoModel = makeInfoLabel();
    _infoImages = makeInfoLabel();

    _linkSelectEngine = makeLink("Select", "Select another engine", SLOT(selectEngine()));
    _linkSelectModel = makeLink("Select", "Select another scenario", SLOT(selectModel()));
    _linkSelectImages = makeLink("Select", "Select image source", SLOT(selectImages()));

    auto panelEngine = new QFrame;
    panelEngine->setProperty("qss-role", "features-panel");
    panelEngine->setLayout(Ori::Gui::layoutV(0, 0, {
        Ori::Gui::layoutH({ Utils::makeTitle("CAFFE ENGINE"), 0, _linkSelectEngine }),
        _infoEngine,
    }));

    auto panelModel = new QFrame;
    panelModel->setProperty("qss-role", "features-panel");
    panelModel->setLayout(Ori::Gui::layoutV(0, 0, {
        Ori::Gui::layoutH({ Utils::makeTitle("CAFFE MODEL"), 0, _linkSelectModel }),
        _infoModel,
    }));

    auto panelImages = new QFrame;
    panelImages->setProperty("qss-role", "features-panel");
    panelImages->setLayout(Ori::Gui::layoutV(0, 0, {
        Ori::Gui::layoutH({ Utils::makeTitle("IMAGE SOURCE"), 0, _linkSelectImages }),
        _infoImages,
    }));

    setLayout(Ori::Gui::layoutV(0, 0,
    {
        panelEngine,
        Utils::makeDivider(),
        panelModel,
        Utils::makeDivider(),
        panelImages,
    }));
}

QWidget* FeaturesPanel::makeLink(const QString& text, const QString& tooltip, const char* slot)
{
    auto link = new QLabel(QString("<a href='dummy'><span style='color:#969C9E'>%1</span></a>").arg(text));
    link->setAlignment(Qt::AlignTop | Qt::AlignRight);
    link->setProperty("qss-role", "link");
    link->setToolTip(tooltip);
    connect(link, SIGNAL(linkActivated(QString)), this, slot);
    return link;
}

QLabel* FeaturesPanel::makeInfoLabel()
{
    auto label = new QLabel;
    label->setWordWrap(true);
    label->setProperty("qss-role", "info-label");
    return label;
}

template<typename T>
QVariant selectCurrentViaDialog(const QList<T>& items, QVariant current) {
    QDialog dlg;
    auto layout = new QVBoxLayout;
    QVector<QRadioButton*> flags;
    for (int i = 0; i < items.size(); i++) {
        auto flag = new QRadioButton(items.at(i).title());
        flag->setChecked(current.isValid() && items[i] == current.value<T>());
        layout->addWidget(flag);
        flags << flag;
    }
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dlg.connect(buttons, SIGNAL(accepted()), &dlg, SLOT(accept()));
    dlg.connect(buttons, SIGNAL(rejected()), &dlg, SLOT(reject()));
    layout->addSpacing(12);
    layout->addWidget(buttons);
    dlg.setLayout(layout);
    QVariant ret;
    if (dlg.exec() == QDialog::Accepted) {
        for (int i = 0; i < flags.size(); i++) {
            if (flags.at(i)->isChecked()) {
                ret.setValue(items[i]);
                break;
            }
        }
    }
    return ret;
}

void FeaturesPanel::selectEngine() {
    auto list = AppConfig::programs();
    if (list.isEmpty()) {
        return Utils::infoDlg(tr("Recognition engines not found"));
    }
    QVariant v = selectCurrentViaDialog(list, AppConfig::currentProgram());
    if (v.isValid()) {
        AppConfig::setCurrentProgram(v.value<Program>().uoa);
        updateExperimentConditions();
    }
}

void FeaturesPanel::selectModel() {
    auto list = AppConfig::models();
    if (list.isEmpty()) {
        return Utils::infoDlg(tr("Models not found"));
    }
    QVariant v = selectCurrentViaDialog(list, AppConfig::currentModel());
    if (v.isValid()) {
        AppConfig::setCurrentModel(v.value<Model>().uoa);
        updateExperimentConditions();
    }
}

void FeaturesPanel::selectImages()
{
    auto list = AppConfig::datasets();
    if (list.isEmpty()) {
        return Utils::infoDlg(tr("Image datasets not found"));
    }
    QVariant v = selectCurrentViaDialog(list, AppConfig::currentDataset());
    if (v.isValid()) {
        AppConfig::setCurrentDataset(v.value<Dataset>().auxUoa);
        updateExperimentConditions();
    }
}

void FeaturesPanel::updateExperimentConditions()
{
    static QString NA("N/A");

    QVariant v = AppConfig::currentProgram();
    _infoEngine->setText(v.isValid() ? v.value<Program>().title() : NA);

    v = AppConfig::currentModel();
    _infoModel->setText(v.isValid() ? v.value<Model>().title() : NA);

    v = AppConfig::currentDataset();
    _infoImages->setText(v.isValid() ? v.value<Dataset>().title() : NA);

}

void FeaturesPanel::enableControls(bool on)
{
    _linkSelectEngine->setVisible(on);
    _linkSelectModel->setVisible(on);
    _linkSelectImages->setVisible(on);
}

void FeaturesPanel::experimentStarted()
{
    enableControls(false);
}

void FeaturesPanel::experimentFinished()
{
    enableControls(true);
}
