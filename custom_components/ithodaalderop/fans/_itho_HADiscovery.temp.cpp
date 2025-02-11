// TEMPORARY FILE FOR DEVELOPMENT PURPOSES
// PRETTIFIED

void addHADiscoveryFan(JsonObject obj, const char *name)
{

    char ihtostatustopic[140]{};
    char cmdtopic[140]{};
    char statetopic[140]{};
    char modestatetopic[140]{};
    snprintf(ihtostatustopic, sizeof(ihtostatustopic), "%s%s", systemConfig.mqtt_base_topic, "/ithostatus");
    snprintf(cmdtopic, sizeof(cmdtopic), "%s%s", systemConfig.mqtt_base_topic, "/cmd");
    snprintf(statetopic, sizeof(statetopic), "%s%s", systemConfig.mqtt_base_topic, "/state");
    snprintf(modestatetopic, sizeof(modestatetopic), "%s%s", systemConfig.mqtt_base_topic, "/modestate");

    std::string uniqueId = normalizeUniqueId(std::string(name) + "_fan");
    JsonObject componentJson = obj[const_cast<char *>(uniqueId.c_str())].to<JsonObject>();
    componentJson["name"] = "Fan";
    componentJson["p"] = "fan";
    componentJson["uniq_id"] = const_cast<char *>(uniqueId.c_str());
    componentJson["json_attr_t"] = const_cast<char *>(ihtostatustopic);                // json_attributes_topic
    componentJson["cmd_t"] = const_cast<char *>(cmdtopic);                             // command_topic
    componentJson["pct_cmd_t"] = const_cast<char *>(cmdtopic);                         // percentage_command_topic
    componentJson["pct_stat_t"] = const_cast<char *>(statetopic);                      // percentage_state_topic
    componentJson["stat_val_tpl"] = "{% if value == '0' %}OFF{% else %}ON{% endif %}"; // state_value_template
    componentJson["pr_mode_cmd_t"] = const_cast<char *>(cmdtopic);                     // preset_mode_command_topic
    componentJson["pr_mode_stat_t"] = const_cast<char *>(ihtostatustopic);             // preset_mode_state_topic

    JsonArray modes = componentJson["pr_modes"].to<JsonArray>(); // preset_modes
    modes.add("Low");
    modes.add("Medium");
    modes.add("High");
    modes.add("Auto");
    modes.add("AutoNight");
    modes.add("Timer 10min");
    modes.add("Timer 20min");
    modes.add("Timer 30min");

    std::string actualSpeedLabel;

    const uint8_t deviceID = currentIthoDeviceID();
    // const uint8_t version = currentItho_fwversion();
    const uint8_t deviceGroup = currentIthoDeviceGroup();

    char pr_mode_val_tpl[400]{};
    char pct_cmd_tpl[300]{};
    char pct_val_tpl[100]{};
    int pr_mode_val_tpl_ver = 0;

    if (deviceGroup == 0x07 && deviceID == 0x01) // HRU250-300
    {
        actualSpeedLabel = getStatusLabel(10, ithoDeviceptr);                         //-> {"Absolute speed of the fan (%)", "absolute-speed-of-the-fan_perc"}, of hru250_300.h
        componentJson["pr_mode_cmd_tpl"] = "{\"rfremotecmd\":\"{{value.lower()}}\"}"; // preset_mode_command_template

        strncpy(pct_cmd_tpl, "
            {%% if value > 90 %%}
                {\"rfremotecmd\":\"high\"}
            {%% elif value > 40 %%}
                {\"rfremotecmd\":\"medium\"}
            {%% elif value > 20 %%}
                {\"rfremotecmd\":\"low\"}
            {%% else %%}
                {\"rfremotecmd\":\"auto\"}
            {%% endif %%}", sizeof(pct_cmd_tpl));

        snprintf(pct_val_tpl, sizeof(pct_val_tpl), "{{ value_json['%s'] | int }}", actualSpeedLabel.c_str());
        componentJson["pl_off"] = "{\"rfremotecmd\":\"auto\"}"; // payload_off
        pr_mode_val_tpl_ver = 1;
    }
    else if (deviceGroup == 0x00 && deviceID == 0x03) // HRU350
    {
        actualSpeedLabel = getStatusLabel(0, ithoDeviceptr);                         //-> {"Requested fanspeed (%)", "requested-fanspeed_perc"}, of hru350.h
        componentJson["pr_mode_cmd_tpl"] = "{\"vremotecmd\":\"{{value.lower()}}\"}"; // preset_mode_command_template
        strncpy(pct_cmd_tpl, "
            {%% if value > 90 %%}
                {\"vremotecmd\":\"high\"}
            {%% elif value > 40 %%}
                {\"vremotecmd\":\"medium\"}
            {%% elif value > 20 %%}
                {\"vremotecmd\":\"low\"}
            {%% else %%}
                {\"vremotecmd\":\"auto\"}
            {%% endif %%}", sizeof(pct_cmd_tpl));

        snprintf(pct_val_tpl, sizeof(pct_val_tpl), "{{ value_json['%s'] | int }}", actualSpeedLabel.c_str());
        componentJson["pl_off"] = "{\"vremotecmd\":\"auto\"}"; // payload_off
        pr_mode_val_tpl_ver = 1;
    }
    else if (deviceGroup == 0x00 && deviceID == 0x0D) // WPU
    {
        // tbd
    }
    else if (deviceGroup == 0x00 && (deviceID == 0x0F || deviceID == 0x30)) // Autotemp
    {
        // tbd
    }
    else if (deviceGroup == 0x00 && deviceID == 0x0B) // DemandFlow
    {
        // tbd
    }
    else if ((deviceGroup == 0x00 && (deviceID == 0x1D || deviceID == 0x14 || deviceID == 0x1B)) || systemConfig.itho_pwm2i2c != 1) // assume CVE and HRU200 / or PWM2I2C is off
    {
        if (deviceID == 0x1D) // hru200
        {
            actualSpeedLabel = getStatusLabel(0, ithoDeviceptr); //-> {"Ventilation setpoint (%)", "ventilation-setpoint_perc"}, of hru200.h
        }
        else if (deviceID == 0x14) // cve 0x14
        {
            actualSpeedLabel = getStatusLabel(0, ithoDeviceptr); //-> {"Ventilation level (%)", "ventilation-level_perc"}, of cve14.h
        }
        else if (deviceID == 0x1B) // cve 0x1B
        {
            actualSpeedLabel = getStatusLabel(0, ithoDeviceptr); //-> {"Ventilation setpoint (%)", "ventilation-setpoint_perc"}, of cve1b.h
        }
        if (systemConfig.itho_pwm2i2c != 1)
        {
            pr_mode_val_tpl_ver = 1;
            snprintf(pct_val_tpl, sizeof(pct_val_tpl), "{{ value_json['%s'] | int }}", actualSpeedLabel.c_str());
            strncpy(pct_cmd_tpl, "{% if value > 90 %}{\"vremotecmd\":\"high\"}{% elif value > 40 %}{ \"vremotecmd\":\"medium\"}{% elif value > 20 %}{\"vremotecmd\":\"low\"}{% else %}{\"vremotecmd\":\"auto\"}{% endif %}", sizeof(pct_cmd_tpl));
            componentJson["pr_mode_cmd_tpl"] = "{\"vremotecmd\":\"{{value.lower()}}\"}"; // preset_mode_command_template
            componentJson["pl_off"] = "{\"vremotecmd\":\"auto\"}";                       // payload_off
        }
        else
        {
            strncpy(pct_val_tpl, "{{ (value | int / 2.55) | round | int }}", sizeof(pct_val_tpl));
            strncpy(pct_cmd_tpl, "{{ (value | int * 2.55) | round | int }}", sizeof(pct_cmd_tpl));
            componentJson["pl_off"] = "0"; // payload_off
        }
    }

    if (pr_mode_val_tpl_ver == 0)
    {
        componentJson["pr_mode_cmd_tpl"] = "{%- if value == 'Timer 10min' %}{{'timer1'}}{%- elif value == 'Timer 20min' %}{{'timer2'}}{%- elif value == 'Timer 30min' %}{{'timer3'}}{%- else %}{{value.lower()}}{%- endif -%}";
        // snprintf(pr_mode_val_tpl, sizeof(pr_mode_val_tpl), "{%%- set speed = value_json['%s'] | int %%}{%%- if speed > 219 %%}high{%%- elif speed > 119 %%}medium{%%- elif speed > 19 %%}low{%%- else %%}auto{%%- endif -%%}", actualSpeedLabel.c_str());
        snprintf(pr_mode_val_tpl, sizeof(pr_mode_val_tpl), "{%%- set speed = value_json['%s'] | int %%}{%%- if speed > 90 %%}High{%%- elif speed > 35 %%}Medium{%%- elif speed > 10 %%}Low{%%- else %%}Auto{%%- endif -%%}", actualSpeedLabel.c_str());

        // strncpy(pr_mode_val_tpl, "{%- if value == 'Low' %}{{'low'}}{%- elif value == 'Medium' %}{{'medium'}}{%- elif value == 'High' %}{{'high'}}{%- elif value == 'Auto' %}{{'auto'}}{%- elif value == 'AutoNight' %}{{'autonight'}}{%- elif value == 'Timer 10min' %}{{'timer1'}}{%- elif value == 'Timer 20min' %}{{'timer2'}}{%- elif value == 'Timer 30min' %}{{'timer3'}}{%- endif -%}", sizeof(pr_mode_val_tpl));
    }
    else if (pr_mode_val_tpl_ver == 1)
    {
        snprintf(pr_mode_val_tpl, sizeof(pr_mode_val_tpl), "{%%- set speed = value_json['%s'] | int %%}{%%- if speed > 90 %%}High{%%- elif speed > 35 %%}Medium{%%- elif speed > 10 %%}Low{%%- else %%}Auto{%%- endif -%%}", actualSpeedLabel.c_str());
    }

    componentJson["pct_cmd_tpl"] = pct_cmd_tpl;         // percentage_command_template
    componentJson["pr_mode_val_tpl"] = pr_mode_val_tpl; // preset_mode_value_template
    componentJson["pct_val_tpl"] = pct_val_tpl;         // percentage_value_template
}