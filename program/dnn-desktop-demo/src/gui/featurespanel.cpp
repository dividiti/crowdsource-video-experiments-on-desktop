#include "appconfig.h"
#include "appmodels.h"
#include "appevents.h"
#include "experimentcontext.h"
#include "featurespanel.h"
#include "../ori/OriWidgets.h"

#include <QBoxLayout>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QSpinBox>

FeaturesPanel::FeaturesPanel(ExperimentContext* context, QWidget *parent) : QFrame(parent)
{
    setObjectName("featuresPanel");

    _context = context;
    connect(_context, &ExperimentContext::experimentStarted, this, &FeaturesPanel::experimentStarted);
    connect(_context, &ExperimentContext::experimentFinished, this, &FeaturesPanel::experimentFinished);

    _infoEngine = makeInfoLabel();
    _infoModel = makeInfoLabel();
    _infoImages = makeInfoLabel();
    _infoMode = makeInfoLabel();

    _linkSelectEngine = makeLink("Select", "Select another engine", SLOT(selectEngine()));
    _linkSelectModel = makeLink("Select", "Select another scenario", SLOT(selectModel()));
    _linkSelectImages = makeLink("Select", "Select image source", SLOT(selectImages()));
    _linkSelectMode = makeLink("Select", "Select mode", SLOT(selectMode()));

    auto panelEngine = new QFrame;
    panelEngine->setProperty("qss-role", "features-panel");
    panelEngine->setLayout(Ori::Gui::layoutV(0, 0, {
        Ori::Gui::layoutH({ Ori::Gui::makeTitle("ENGINE"), 0, _linkSelectEngine }),
        _infoEngine,
    }));

    auto panelModel = new QFrame;
    panelModel->setProperty("qss-role", "features-panel");
    panelModel->setLayout(Ori::Gui::layoutV(0, 0, {
        Ori::Gui::layoutH({ Ori::Gui::makeTitle("MODEL"), 0, _linkSelectModel }),
        _infoModel,
    }));

    auto panelImages = new QFrame;
    panelImages->setProperty("qss-role", "features-panel");
    panelImages->setLayout(Ori::Gui::layoutV(0, 0, {
        Ori::Gui::layoutH({ Ori::Gui::makeTitle("IMAGE SOURCE"), 0, _linkSelectImages }),
        _infoImages,
    }));

    auto panelMode = new QFrame;
    panelMode->setProperty("qss-role", "features-panel");
    panelMode->setLayout(Ori::Gui::layoutV(0, 0, {
        Ori::Gui::layoutH({ Ori::Gui::makeTitle("MODE"), 0, _linkSelectMode }),
        _infoMode,
    }));

    auto settingsPanel = new QFrame;
    settingsPanel->setLayout(Ori::Gui::layoutV(0, 0, {
        panelEngine,
        Ori::Gui::makeDivider(),
        panelModel,
        Ori::Gui::makeDivider(),
        panelImages,
    }));

    setLayout(Ori::Gui::layoutV(0, 0,
    {
        panelMode,
        Ori::Gui::makeDivider(),
        settingsPanel
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
static QVariant selectCurrentViaDialog(const QList<T>& items, QVariant current) {
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

static QVariant selectEngineAndBatchSizeViaDialog() {
    QVariant ret;

    const Mode::Type m = AppConfig::currentModeType();

    auto items = AppConfig::programs(m);
    if (items.isEmpty()) {
        AppEvents::info("Recognition engines not found");
        return ret;
    }

    auto current = AppConfig::currentProgram(m);
    QDialog dlg;
    auto layout = new QVBoxLayout;

    QVector<QRadioButton*> flags;
    for (int i = 0; i < items.size(); i++) {
        auto flag = new QRadioButton(items.at(i).title());
        flag->setChecked(current.isValid() && items[i] == current.value<Program>());
        layout->addWidget(flag);
        flags << flag;
    }

    auto batchSize = new QSpinBox;
    batchSize->setMinimum(1);
    batchSize->setSingleStep(1);
    batchSize->setValue(AppConfig::batchSize());

    auto batchSizeWidget = new QWidget;
    auto batchSizeLayout = new QHBoxLayout;
    batchSizeLayout->addWidget(new QLabel("Batch size: "));
    batchSizeLayout->addWidget(batchSize);
    batchSizeWidget->setLayout(batchSizeLayout);

    if (Mode::Type::CLASSIFICATION == m) {
        auto line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        layout->addSpacing(12);
        layout->addWidget(line);
    } else {
        batchSizeWidget->setVisible(false);
    }

    layout->addWidget(batchSizeWidget);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dlg.connect(buttons, SIGNAL(accepted()), &dlg, SLOT(accept()));
    dlg.connect(buttons, SIGNAL(rejected()), &dlg, SLOT(reject()));
    layout->addWidget(buttons);

    dlg.setLayout(layout);
    if (dlg.exec() == QDialog::Accepted) {
        if (Mode::Type::CLASSIFICATION == m) {
            AppConfig::setBatchSize(batchSize->value());
        }
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
    QVariant v = selectEngineAndBatchSizeViaDialog();
    if (v.isValid()) {
        AppConfig::setCurrentProgram(v.value<Program>().targetUoa);
        updateExperimentConditions();
    }
}

void FeaturesPanel::selectModel() {
    auto list = AppConfig::models();
    if (list.isEmpty()) {
        return AppEvents::info("Models not found");
    }
    QVariant v = selectCurrentViaDialog(list, AppConfig::currentModel());
    if (v.isValid()) {
        AppConfig::setCurrentModel(v.value<Model>().uoa);
        updateExperimentConditions();
    }
}

void FeaturesPanel::selectImages() {
    auto list = AppConfig::datasets();
    if (list.isEmpty()) {
        return AppEvents::info("Image datasets not found");
    }
    QVariant v = selectCurrentViaDialog(list, AppConfig::currentDataset());
    if (v.isValid()) {
        AppConfig::setCurrentDataset(v.value<Dataset>().valUoa);
        updateExperimentConditions();
    }
}

void FeaturesPanel::selectMode() {
    auto list = AppConfig::modes();
    if (list.isEmpty()) {
        return AppEvents::info("No run modes found");
    }
    QVariant v = selectCurrentViaDialog(list, AppConfig::currentMode());
    if (v.isValid()) {
        Mode m = v.value<Mode>();
        AppConfig::setCurrentMode(m.type);
        _context->notifyModeChanged(m);
        updateExperimentConditions();
    }
}

void FeaturesPanel::updateExperimentConditions() {
    static const QString NA("N/A");

    auto m = AppConfig::currentModeType();

    QVariant v = AppConfig::currentProgram(m);
    _infoEngine->setText(v.isValid() ? v.value<Program>().title() : NA);

    v = AppConfig::currentModel(m);
    _infoModel->setText(v.isValid() ? v.value<Model>().title() : NA);

    v = AppConfig::currentDataset(m);
    _infoImages->setText(v.isValid() ? v.value<Dataset>().title() : NA);

    _infoMode->setText(Mode(m).title());
}

void FeaturesPanel::enableControls(bool on)
{
    _linkSelectEngine->setVisible(on);
    _linkSelectModel->setVisible(on);
    _linkSelectImages->setVisible(on);
    _linkSelectMode->setVisible(on);
}

void FeaturesPanel::experimentStarted()
{
    enableControls(false);
}

void FeaturesPanel::experimentFinished()
{
    enableControls(true);
}
