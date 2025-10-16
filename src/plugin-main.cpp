#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QMainWindow>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>

#include "settings.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-xbox-zoom-pan", "en-US")

// Controller thread
extern void start_controller_thread();
extern void stop_controller_thread();

// Transformer control
extern void set_target_source(const char* name);
extern void set_enabled(bool e);
extern void set_speeds(float z, float p);
extern void set_deadzone(float d);

static QDockWidget* g_dock = nullptr;
static QComboBox*   g_srcBox = nullptr;

static void populate_sources(QComboBox* box)
{
    box->clear();
    obs_source_t* scene = obs_frontend_get_current_scene();
    if (!scene) return;

    obs_scene_t* s = obs_scene_from_source(scene);
    obs_scene_enum_items(s,
        [](obs_scene_t*, obs_sceneitem_t* item, void* user) {
            QComboBox* b = static_cast<QComboBox*>(user);
            obs_source_t* src = obs_sceneitem_get_source(item);
            b->addItem(obs_source_get_name(src));
            return true;
        },
        box);

    obs_source_release(scene);

    if (box->count() > 0) {
        box->setCurrentIndex(0);
        set_target_source(box->currentText().toUtf8().constData());
    }
}

static void build_dock()
{
    if (g_dock) return;

    QWidget* w = new QWidget();
    QVBoxLayout* v = new QVBoxLayout(w);

    g_srcBox = new QComboBox();
    populate_sources(g_srcBox);
    QObject::connect(g_srcBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [] (int) {
            if (!g_srcBox) return;
            set_target_source(g_srcBox->currentText().toUtf8().constData());
        });

    QCheckBox* en = new QCheckBox("Enable");
    en->setChecked(g_settings.enabled);
    QObject::connect(en, &QCheckBox::toggled, [] (bool c) { set_enabled(c); });

    QDoubleSpinBox* zs = new QDoubleSpinBox(); zs->setRange(0.1, 5.0); zs->setSingleStep(0.1); zs->setValue(g_settings.zoomSpeed);
    QDoubleSpinBox* ps = new QDoubleSpinBox(); ps->setRange(0.1, 5.0); ps->setSingleStep(0.1); ps->setValue(g_settings.panSpeed);
    QDoubleSpinBox* dz = new QDoubleSpinBox(); dz->setRange(0.0, 0.5); dz->setSingleStep(0.05); dz->setValue(g_settings.deadzone);

    auto update_speeds = [=]() { set_speeds((float)zs->value(), (float)ps->value()); };
    QObject::connect(zs, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double){ update_speeds(); });
    QObject::connect(ps, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double){ update_speeds(); });
    QObject::connect(dz, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double){ set_deadzone((float)dz->value()); });

    v->addWidget(new QLabel("Source:"));
    v->addWidget(g_srcBox);
    v->addWidget(en);
    v->addWidget(new QLabel("Zoom speed:")); v->addWidget(zs);
    v->addWidget(new QLabel("Pan speed:"));  v->addWidget(ps);
    v->addWidget(new QLabel("Dead-zone:"));  v->addWidget(dz);
    v->addStretch();

    g_dock = new QDockWidget("Xbox Zoom/Pan");
    g_dock->setObjectName("obs_xbox_zoom_pan_dock");
    g_dock->setWidget(w);

    QMainWindow* main = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    main->addDockWidget(Qt::RightDockWidgetArea, g_dock);
}

static void on_frontend_event(enum obs_frontend_event ev, void*)
{
    switch (ev) {
        case OBS_FRONTEND_EVENT_FINISHED_LOADING:
            build_dock();
            break;
        case OBS_FRONTEND_EVENT_SCENE_CHANGED:
        case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
        case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:
            if (g_srcBox) populate_sources(g_srcBox);
            break;
        default: break;
    }
}

bool obs_module_load()
{
    load_plugin_settings();
    start_controller_thread();
    obs_frontend_add_event_callback(on_frontend_event, nullptr);
    return true;
}

void obs_module_unload()
{
    obs_frontend_remove_event_callback(on_frontend_event, nullptr);

    if (g_dock) {
        if (g_dock->parentWidget())
            static_cast<QMainWindow*>(g_dock->parentWidget())->removeDockWidget(g_dock);
        delete g_dock; g_dock = nullptr; g_srcBox = nullptr;
    }

    save_plugin_settings();
    stop_controller_thread();
}
