static const g_versionLink[] = "https://api.github.com/repos/"
    + "ReDeathmatch/ReDeathmatch_AMXX"
    + "/releases/latest"

static bool: redm_update_notify

Updater_Init() {
    bind_pcvar_num(
        create_cvar(
            "redm_update_notify", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Notify for new ReDM update."
        ),
        redm_update_notify
    )

    register_concmd(
        "redm_update_check", "ConCmd_redm_update_check",
        ADMIN_MAP,
        "Checks ReDM update."
    )
}

public ConCmd_redm_update_check(const player, const level, const commandId) {
    SetGlobalTransTarget(player)
      
    if (!cmd_access(player, level, commandId, 1))
        return PLUGIN_HANDLED

    Request_NewVersion(g_versionLink)

    return PLUGIN_HANDLED
}

Updater_Natives() {
    set_module_filter("@ModuleFilter")
    set_native_filter("@NativeFilter")
}

@ModuleFilter(const library[], LibType: type) {
    return strcmp("easy_http", library) == 0 ? PLUGIN_HANDLED : PLUGIN_CONTINUE
}

@NativeFilter(const nativeName[], index, trap) {
    if (strncmp(nativeName, "ezhttp_", 7) == 0)
        return PLUGIN_HANDLED

    if (strncmp(nativeName, "ezjson_", 7) == 0)
        return PLUGIN_HANDLED

    return PLUGIN_CONTINUE
}

Updater_Check() {
    if (!redm_update_notify)
        return

    new bool: isManualBuild = (strfind(REDM_VERSION, "manual") != -1)
    if (isManualBuild)
        return

    if (is_module_loaded("Amxx Easy Http") == -1) {
        LogMessageEx(Warning, "The `AmxxEasyHttp` module is not loaded! The new version cannot be verified.")
        LogMessageEx(Warning, "Please install AmxxEasyHttp: `https://github.com/Next21Team/AmxxEasyHttp` or disable update checks (`redm_update_notify 0`).")

        return
    }

    Request_NewVersion(g_versionLink)
}

static Request_NewVersion(const link[]) {
    ezhttp_get(link, "@RequestCallback_NewVersion")
}

@RequestCallback_NewVersion(EzHttpRequest: request_id) {
    if (ezhttp_get_error_code(request_id) != EZH_OK) {
        new error[64]
        ezhttp_get_error_message(request_id, error, charsmax(error))

        LogMessageEx(Warning, "RequestCallback_NewVersion(%i): Response error: %s", request_id, error)
        return
    }

    new response[8192]
    ezhttp_get_data(request_id, response, charsmax(response))

    if (contain(response, "tag_name") == -1) {
        LogMessageEx(Warning, "RequestCallback_NewVersion: Wrong response! (don't contain `tag_name`). res=`%s`", response)
        return
    }

    new EzJSON: json = ezjson_parse(response)
    if (json == EzInvalid_JSON) {
        LogMessageEx(Warning, "RequestCallback_NewVersion: Can't parse response JSON!")
        goto END
    }

    new tag_name[32]
    ezjson_object_get_string(json, "tag_name", tag_name, charsmax(tag_name))

    if (CmpVersions(REDM_VERSION, tag_name) >= 0)
        goto END

    new html_url[256]
    ezjson_object_get_string(json, "html_url", html_url, charsmax(html_url))

    NotifyUpdate(tag_name, html_url)

    END:
    ezjson_free(json)
}

static NotifyUpdate(const newVersion[], const URL[]) {
    LogMessageEx(Info, "^n^t ReDeathmatch (%s) has update! New version `%s`.^n\
        Download link: `%s`", REDM_VERSION, newVersion, URL
    )
}

static stock CmpVersions(const a[], const b[]) {
    new segmentsA[32][32]
    new segmentsB[32][32]

    new countA = explode_string(
        a[!isdigit(a[0]) ? 1 : 0],
        ".",
        segmentsA, sizeof segmentsA, charsmax(segmentsA[])
    )

    new countB = explode_string(
        b[!isdigit(b[0]) ? 1 : 0],
        ".",
        segmentsB, sizeof segmentsB, charsmax(segmentsB[])
    )

    for(new i, l = min(countA, countB); i < l; i++) {
        new diff = strtol(segmentsA[i]) - strtol(segmentsB[i])
        if (diff)
            return diff
    }

    return countA - countB
}
