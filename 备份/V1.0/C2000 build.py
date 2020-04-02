#!/usr/bin/env python
# --*-- coding: utf-8

#C2000编译脚本
#用于解决CCS3.3在WIN10下无法使用问题，python版本需要3.5以上
#版本：V100
#日期：2019-03-29
#作者: liansh

import os
import sys
import subprocess
import concurrent.futures   #线程池支持
import functools
from itertools import repeat

class PJT(object):
    def __init__(self, project_dir):
        self.proj_dir = project_dir
        self.pjt_dir = None
        self.pjt_data = None
        
        self.lib_file = []
        self.c_file = []
        self.cmd_file = []
        
        #self.inlude_dir = ''
        self.obj_dir_debug = ''
        self.obj_dir_release = ''
        self.tmp_dir = os.path.join(self.proj_dir, 'tmp')
        self.output_dir_debug = os.path.join(self.proj_dir, 'debug')
        self.output_dir_release = os.path.join(self.proj_dir, 'release')

        self.initial_build_cmd = []
        self.final_build_cmd = []

        self.compiler_config_debug = ''
        self.linker_config_debug = ''
        self.compiler_config_release = ''
        self.linker_config_release = ''

        self.status = False

        if self.__open_pjt():
            self.__analyze_pjt()
            self.status = True
        else:
            print('pjt文件异常')

    def __open_pjt(self):
        """查找并打开.pjt文件"""
        for file in os.listdir(self.proj_dir):
            if '.pjt' in file.lower():          #lower全部转小写
                self.pjt_dir = os.path.join(self.proj_dir, file)
                if os.path.exists(self.pjt_dir):
                    with open(self.pjt_dir, 'r', encoding='utf-8') as file:
                        self.pjt_data = file.readlines()
                    return(True)
        return(False)

    def __analyze_pjt(self):
        """解析工程文件"""
        #解析文件(从[Source Files]项开始)
        for i in self.pjt_data[self.pjt_data.index('[Source Files]\n')+1 : ]:
            if 'Source' not in i:
                break                           #文件解析完成
            else:
                file_name = i[i.find('"')+1 : -2].lower()
                if '.lib' in file_name:         #lib文件
                    self.lib_file.append(os.path.join(self.proj_dir, file_name))
                elif '.cmd' in file_name:       #cmd文件
                    self.cmd_file.append(os.path.join(self.proj_dir, file_name))
                else:                           #其它文件
                    self.c_file.append(os.path.join(self.proj_dir, file_name))
        #解析编译参数(compiler)
        #replace(a, b, c) a被替换字符，b替换字符，c最大替换次数
        row = self.pjt_data.index('["Compiler" Settings: "Debug"]\n')
        self.compiler_config_debug = self.pjt_data[row+1][8 : -1].replace('$(Proj_dir)', self.proj_dir, 10)
        row = self.pjt_data.index('["Compiler" Settings: "Release"]\n')
        self.compiler_config_release = self.pjt_data[row+1][8 : -1].replace('$(Proj_dir)', self.proj_dir, 10)        

        #解析链接参数(Linker)
        row = self.pjt_data.index('["Linker" Settings: "Debug"]\n')
        self.linker_config_debug = self.pjt_data[row+1][8 : -1].replace('$(Proj_dir)', self.proj_dir, 10)
        row = self.pjt_data.index('["Linker" Settings: "Release"]\n')
        self.linker_config_release = self.pjt_data[row+1][8 : -1].replace('$(Proj_dir)', self.proj_dir, 10)

        #OBJ目录
        start = self.compiler_config_debug.find('-fr')
        end = start + 4 + self.compiler_config_debug[start+4 : ].find('"')
        if start >= 0:        #有找到OBJ参数设置值
            self.obj_dir_debug = self.compiler_config_debug[start+4 : end]
        start = self.compiler_config_release.find('-fr')
        end = start + 4 + self.compiler_config_release[start+4 : ].find('"')
        if start >= 0:
            self.obj_dir_release = self.compiler_config_release[start+4 : end]

        #前置(后置)任务
        row = self.pjt_data.index('["Debug" Settings]\n')
        for i in self.pjt_data[row+1 : ]:
            if '\n' == i:
                return
            if 'FinalBuildCmd' in i:        #后置任务
                self.final_build_cmd.append(i[ : -1])
            elif 'InitialBuildCmd' in i:    #前置任务
                self.initial_build_cmd.append(i[ : -1])

#编译器
#work_dir : 工程所在目录
#c2000_dir: C2000软件包所在目录
#mode     : 编译模式(DEBUG, RELEASE)
#works    : 多线程编译线程数
class Compiler(object):
    def __init__(self, work_dir, c2000_dir, mode='DEBUG', works=4):
        self.mode = mode.upper()
        self.lkf_dir = None
        self.build_log = []
        self.c2000_dir = c2000_dir
        try:
            self.max_works = int(works)
        except:
            self.max_works = 4

        self.proj = PJT(work_dir)

        os.chdir(work_dir)

    def run(self):
        if not self.proj.status:
            return

        #检查是否有临时目录，如果没有则创建，并清空临时目录
        #/q不需要手动确认 /f删除只读文件
        if not os.path.exists(self.proj.tmp_dir):
            os.mkdir(self.proj.tmp_dir)
        subprocess.getstatusoutput('del /q /f ' + self.proj.tmp_dir)

        #前置任务
        self.initial(self.proj)

        #编译
        if not self.build(self.proj, self.mode):
            return

        #lvk文件生成
        self.create_lkf(self.proj, self.mode, self.build_log)

        #连接
        self.linker()

        #后置任务
        self.final(self.proj)

    def create_lkf(self, proj, mode, build_log):
        """创建lkf文件，并写入内容"""
        if 'DEBUG' == mode.upper():
            linker_config = proj.linker_config_debug
        else:
            linker_config = proj.linker_config_release

        #CCS3.3自带lib目录加入
        linker_config += ' -i"' + os.path.join(self.c2000_dir, 'xdais\lib') + '"'
        linker_config += ' -i"' + os.path.join(self.c2000_dir, 'cgtools\lib') + '"\n'

        self.lkf_dir = os.path.join(proj.tmp_dir, mode+'.lkf')
        with open(self.lkf_dir, 'w', encoding='utf-8') as file:
            #先写入Linker参数,加z意义不明,CCS中找不到相关设置
            #但是CS生成的文件中有此参数
            file.writelines('-z ' + linker_config)
            for i in build_log:
                file.writelines(i)                  #再写入编译文件目录(编译日志)
            #写入lib文件
            for i in proj.lib_file:
                file.writelines('"' + i + '"\n')    #再写入编译文件目录(编译日志)
            #写入cmd文件
            for i in proj.cmd_file:
                file.writelines('"' + i + '"\n')    #再写入编译文件目录(编译日志)

    def build(self, proj, mode):
        """编译"""
        if 'DEBUG' == mode:
            build_config = proj.compiler_config_debug + ' '
            obj_dir = proj.obj_dir_debug
        else:
            build_config = proj.compiler_config_release + ' '
            obj_dir = proj.obj_dir_release

        bin_dir = '"' + os.path.join(self.c2000_dir, 'cgtools\\bin\\cl2000') + '" '
        lkf_dir = ''        #'-@"' + self.lkf_dir + '" '
        cmd = bin_dir + build_config + lkf_dir

        #使用线程池来加速文件编译
        #在使用map时，线程池返回顺序与启动顺序相同，可以确保.lkf文件中OBJ文件顺序与实际待编译文件顺序一致
        #线程池中线程同时传入固定参数及list参数时，可以使用functools.partial或repeat来指定固定参数
        #但是注意，必须是可变参数在前，固定参数在后
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.max_works) as executor:
            #for status, output in zip(proj.c_file, executor.map(functools.partial(cl2000_shell, cc=cmd), proj.c_file)):
            for status, output in zip(proj.c_file, executor.map(cl2000_shell, proj.c_file, repeat(cmd))):
                if 0 == output[0]:
                    obj_file = os.path.join(obj_dir, status[status.rfind('\\')+1 : status.rfind('.')]) + '.obj'
                    self.build_log.append('"' + obj_file + '"\n')
                    print(status + ' success --' + mode)
                else:
                    print(output[1])
                    return(False)
        return(True)

    def linker(self):
        """链接"""
        bin_dir = '"' + os.path.join(self.c2000_dir, 'cgtools\\bin\\cl2000') + '" '
        cmd = bin_dir + '-@"' + self.lkf_dir + '"'
        status, output = subprocess.getstatusoutput(cmd)
        print('[Linking...] ' +  self.lkf_dir)
        print(output)

    def initial(self, proj):
        """前置任务"""
        pass
    def final(self, proj):
        """后置任务"""
        for i in proj.final_build_cmd:
            if 'hex2000' in i:
                cmd = '"' + os.path.join(self.c2000_dir, 'cgtools\\bin\\hex2000') + '"'
                cmd += ' ' + i[22 : ]
            else:
                cmd = i[14 : ]
            status, output = subprocess.getstatusoutput(cmd)
            print(output)

#bin：bin文件目录
#lkf: lkf文件目录
#cfg: build配置
#file:待编译文件
def cl2000_shell(file, cc):
    """CMD执行指令"""
    cmd = cc + '"' + file + '"'
    status, output = subprocess.getstatusoutput(cmd)
    return(status, output)

if __name__ == '__main__':
    import time
    start_time = time.time()
    if len(sys.argv) > 4:       #正常传入参数
        cc = Compiler(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
        cc.run()
    else:                       #无参数,默认调试
        print("Parameter cannot be empty")
    end_time = time.time()
    print("编译用时: {0:.2f} 秒".format(end_time - start_time))
    sys.exit()
