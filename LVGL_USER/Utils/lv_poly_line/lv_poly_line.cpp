#include "Utils/lv_poly_line/lv_poly_line.h"

lv_poly_line::lv_poly_line(lv_obj_t *par)
        : current_index(0), styleLine(nullptr), parent(par) {
}

lv_poly_line::~lv_poly_line() {
    for (auto iter: poly_line) {
        if (iter.line) {
            lv_obj_del(iter.line);//delete line obj
        }

        //swap()函数会交换两个vector,这里是将points清空
        decltype(iter.points) vec;
        iter.points.swap(vec);
    }
}
/**
 * @brief 添加一条线 create single_line and add to  poly_line vector
 * 
 */
void lv_poly_line::add_line() {
    //create line obj
    lv_obj_t *line = lv_line_create(parent);
    lv_obj_remove_style_all(line);
    //add style
    if (styleLine != nullptr) {
        lv_obj_add_style(line, styleLine, 0);
    }

    //push to vector
    single_line_t single_line;
    single_line.line = line;
    poly_line.push_back(single_line);

    LV_LOG_INFO("line: %p, parent: %p", line, parent);
}
/**
 * @brief 重新绘制poly_line
 * 
 */
void lv_poly_line::refresh_line() {
    //polyline 段数
    size_t size = poly_line.size();

    for (size_t i = 0; i < size; i++) {
        single_line_t *single_line = &poly_line[i];
        const lv_point_t *points = get_points(single_line);
        //根据points绘制single_line
        lv_line_set_points(single_line->line, points, (uint16_t) single_line->points.size());
    }
}

/**
 * @brief 绘制poly_line,并新增提一条(index = current_index)single_line ,开始收集point绘制这条single_line
 * 
 */
void lv_poly_line::start() {
    // 增加一条新的line obj，并绘制所有已存在的single_line
    // 这个判断与reset中的有关，reset中只是清空了single_line的points的元素，并没有clear poly_line的元素，所以poly_line的size不变，一经分配就不释放.所以不一定需要new 一个line obj
    if (current_index >= poly_line.size()) {
        LV_LOG_INFO("current_index[%d] >= poly_line.size()[%d]", current_index, poly_line.size());
        add_line();
        refresh_line();
    }

    LV_LOG_INFO("show poly_line[%d]", current_index);
    //get current singleline
    single_line_t *single_line = &poly_line[current_index];

    //show line
    lv_obj_clear_flag(single_line->line, LV_OBJ_FLAG_HIDDEN);
    //清空single_line的points
    lv_line_set_points(single_line->line, nullptr, 0);
}

/**
 * @brief 向当前的single_line添加点
 * 
 * @param point 
 */
void lv_poly_line::append(const lv_point_t *point) {
    if (poly_line.size() == 0) {
        LV_LOG_WARN("poly_line.size() = 0");
        return;
    }

    LV_LOG_INFO("poly_line[%d]: (%d, %d)", current_index, point->x, point->y);
    poly_line[current_index].points.push_back(*point);
}
/**
 * @brief 将point添加到最后一条(index = current_index - 1 )single_line,并且绘制该single_line
 *        因为(index = current_index - 1 )的single_line已经描绘完了。
 * 
 * @param point 
 */
void lv_poly_line::append_to_end(const lv_point_t *point) {
    //获取最后一条single_line的地址
    single_line_t *single_line = get_end_line();
    if (single_line == nullptr) {
        LV_LOG_ERROR("failed");
        return;
    }
    //push point to single_line
    single_line->points.push_back(*point);

    LV_LOG_INFO("end_line: (%d, %d), size = %d", point->x, point->y, single_line->points.size());

    const lv_point_t *points = get_points(single_line);
    //根据points绘制single_line
    lv_line_set_points(single_line->line, points, (uint16_t) single_line->points.size());
}

/**
 * @brief 根据points绘制当前的single_line，并结束当前的single_line的绘制 (current_index++)
 * 
 */
void lv_poly_line::stop() {
    //size 为0,说明没有single_line
    if (poly_line.size() == 0) {
        LV_LOG_ERROR("poly_line.size() = 0");
        return;
    }
    //获取当前的single_line
    single_line_t *single_line = &poly_line[current_index];
    //根据points绘制single_line
    const lv_point_t *points = get_points(single_line);
    lv_line_set_points(single_line->line, points, (uint16_t) single_line->points.size());

    LV_LOG_INFO("poly_line[%d] size = %d", current_index, single_line->points.size());
    //
    current_index++;
}

/**
 * @brief current_index清0，poly_line的所有single_line的points清空，但是poly_line.size不变
 * 
 */
void lv_poly_line::reset() {
    current_index = 0;
    size_t size = poly_line.size();
    LV_LOG_INFO("poly_line.size() = %d", size);
    LV_LOG_INFO("reset current_index");

    //这里只是clear了single_line的points的元素，并没有clear poly_line的元素，所以poly_line的size不变，一经分配就不释放.
    for (size_t i = 0; i < size; i++) {
        single_line_t *single_line = &poly_line[i];
        //清空single_line的points
        lv_line_set_points(single_line->line, nullptr, 0);
        single_line->points.clear();//清空vector的元素
        lv_obj_add_flag(single_line->line, LV_OBJ_FLAG_HIDDEN);//hidden line
    }
} 
/**
 * @brief  获取poly_line的最后一条single_line的地址
 *         index为current_index - 1 
 * 
 * @return lv_poly_line::single_line_t* 
 */
lv_poly_line::single_line_t *lv_poly_line::get_end_line() {
    size_t size = poly_line.size();
    if (size == 0) {
        LV_LOG_ERROR("NOT found");
        return nullptr;
    } else {
        LV_LOG_INFO("end_line index = %d", current_index - 1);
    }

    return &poly_line[current_index - 1];
}
/**
 * @brief 获取single_line的起始点的地址
 * 
 * @param single_line 
 * @return single_line为空就return nullptr
 */
const lv_point_t *lv_poly_line::get_points(single_line_t *single_line) {
    const lv_point_t *points = nullptr;

    if (single_line->points.size()) {
        points = &single_line->points[0];
    } else {
        LV_LOG_WARN("NOT found");
    }

    return points;
}
/**
 * @brief 获取poly_line的最后一条(current_index - 1 )single_line的最后一个point的地址
 * 
 * @param point 
 * @return true 
 * @return false 
 */
bool lv_poly_line::get_end_point(lv_point_t *point) {
    
    single_line_t *single_line = get_end_line();
    if (single_line == nullptr) {
        LV_LOG_ERROR("end line NOT found");
        return false;
    }

    if (single_line->points.size() == 0) {
        LV_LOG_ERROR("single_line->points is EMPTY");
        return false;
    }
    //获得最后一个point的地址
    *point = single_line->points.back();
    return true;
}
