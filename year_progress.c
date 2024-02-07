#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/elements.h>

#include <year_progress_icons.h>

#define TAG "YearProgress"

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} AppEvent;

typedef struct {
    FuriMessageQueue* event_queue;
    Gui* gui;
    ViewPort* view;
    FuriMutex* mutex;
    FuriTimer* timer;
} YearProgressApp;

// 获取指定年的总秒数
uint32_t get_year_total_sec(uint16_t year) {
    return furi_hal_rtc_get_days_per_year(year) * 24 * 60 * 60;
}

// 获取指定年起始时间戳
uint32_t get_year_start_ts(uint16_t year) {
    FuriHalRtcDateTime year_start_dt = {
        .year = year,
        .month = 1,
        .day = 1,
        .hour = 0,
        .minute = 0,
        .second = 0,
    };
    return furi_hal_rtc_datetime_to_timestamp(&year_start_dt);
}

static void year_progress_render_cb(Canvas* const canvas, void* ctx) {
    YearProgressApp* instance = ctx;
    furi_mutex_acquire(instance->mutex, FuriWaitForever);

    FuriHalRtcDateTime curr_dt;
    furi_hal_rtc_get_datetime(&curr_dt);
    uint32_t curr_ts = furi_hal_rtc_datetime_to_timestamp(&curr_dt);
    uint32_t year_start_ts = get_year_start_ts(curr_dt.year);
    uint32_t year_total_sec = get_year_total_sec(curr_dt.year);

    // 计算今年 1月1日 00:00:00 到当前所走过的秒数
    uint32_t year_pass_sec = curr_ts - year_start_ts;

    // 计算当前秒在一年中的百分比
    double year_progress = (double)year_pass_sec / year_total_sec;

    // FURI_LOG_D(
    //     TAG, "y_prog=%lu/%lu  percent=%.6f%%", year_pass_sec, year_total_sec, year_progress * 100);

    char buffer[20];

    // 文案
    canvas_draw_icon(canvas, 0, 0, &I_text_of_save_time);

    // 年份
    canvas_set_font(canvas, FontPrimary);
    snprintf(buffer, sizeof(buffer), "Year %u", curr_dt.year);
    canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignCenter, buffer);

    // 进度条
    elements_progress_bar(canvas, 0, 35, 128, year_progress);

    // 百分比数字
    canvas_set_font(canvas, FontBigNumbers);
    snprintf(buffer, sizeof(buffer), "%.6f", year_progress * 100);
    uint16_t num_str_width = canvas_string_width(canvas, buffer);
    canvas_draw_str(canvas, 4, 64, buffer);
    canvas_draw_icon(canvas, num_str_width + 6, 50, &I_Percent_10x14);

    furi_mutex_release(instance->mutex);
}

static void year_progress_input_cb(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    AppEvent event = {
        .type = EventTypeKey,
        .input = *input_event,
    };
    furi_message_queue_put(event_queue, &event, 0);
}

static void tick_timer_cb(void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    AppEvent event = {
        .type = EventTypeTick,
    };
    furi_message_queue_put(event_queue, &event, 0);
}

YearProgressApp* year_progress_alloc() {
    YearProgressApp* app = malloc(sizeof(YearProgressApp));

    app->event_queue = furi_message_queue_alloc(8, sizeof(AppEvent));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->timer = furi_timer_alloc(tick_timer_cb, FuriTimerTypePeriodic, app->event_queue);
    app->gui = furi_record_open(RECORD_GUI);

    app->view = view_port_alloc();
    view_port_input_callback_set(app->view, year_progress_input_cb, app->event_queue);
    view_port_draw_callback_set(app->view, year_progress_render_cb, app);
    gui_add_view_port(app->gui, app->view, GuiLayerFullscreen);

    return app;
}

void year_progress_free(YearProgressApp* app) {
    furi_timer_free(app->timer);
    gui_remove_view_port(app->gui, app->view);
    view_port_free(app->view);
    furi_record_close(RECORD_GUI);
    furi_mutex_free(app->mutex);
    furi_message_queue_free(app->event_queue);

    free(app);
}

int32_t year_progress_app(void* p) {
    UNUSED(p);
    YearProgressApp* app = year_progress_alloc();

    // 定时器以 1s 触发
    furi_timer_start(app->timer, furi_kernel_get_tick_frequency());

    AppEvent event;
    while(furi_message_queue_get(app->event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        if(event.type == EventTypeKey) {
            // 短按 BackKey 退出 App
            if(event.input.key == InputKeyBack && event.input.type == InputTypeShort) {
                break;
            }
        } else if(event.type == EventTypeTick) {
            view_port_update(app->view);
        }
    }

    furi_timer_stop(app->timer);
    year_progress_free(app);

    return 0;
}
