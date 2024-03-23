target("marl")
_config_project({
	project_kind = "static"
})
add_includedirs("include", {public = true})
add_files("src/*.c")
on_load(function(target)
    local src_path = path.join(os.scriptdir(), "src")
    for _, filepath in ipairs(os.files(path.join(src_path, "*.cpp"))) do
        local file_name = path.filename(filepath)
        if not (file_name:find("_bench") or file_name:find("_test")) then
            target:add("files", filepath)
        end
    end
end)
target_end()