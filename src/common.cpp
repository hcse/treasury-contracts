#include "common.hpp";

bool common::is_paused () {
    config_table      config_s (get_self(), get_self().value);
    Config c = config_s.get_or_create (get_self(), Config());   
    check (c.ints.find ("paused") != c.ints.end(), "Contract does not have a pause configuration. Assuming it is paused. Please contact administrator.");
        
    uint64_t paused = c.ints.at("paused");
    return paused == 1;
}
