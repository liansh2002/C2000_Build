<!--
 * @Author: your name
 * @Date: 2020-04-02 13:50:19
 * @LastEditTime: 2020-04-02 14:05:04
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \C2000 Compiler\README.md
 -->
# C2000_Build
多线程编译CCS3.3工程。Python3.7

VSCode task 配置
{
    // See https://go.microsoft.com/fwlink/?LinkId=733558
    // for the documentation about the tasks.json format
    "version": "2.0.0",
    "tasks": [
        {
            "label": "C2000 Compiler",
            "type": "shell",
            "command": "python",
            "group": {
                "kind": "build",
                "isDefault": true,
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": true,
                "panel": "shared",
                "showReuseMessage": false,
                "clear": true
            },
            "args": [
                "E:\\Python\\Project\\C2000 Compiler\\C2000 build.py",
                "${workspaceRoot}",
                "E:\\Python\\Project\\C2000 Compiler\\C2000",
                "DEBUG",
                "4",
            ]
        }
    ]
}