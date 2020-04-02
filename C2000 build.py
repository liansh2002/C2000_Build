#!/usr/bin/env python
# --*-- coding: utf-8
"""
C2000编译脚本
用于解决CCS3.3在WIN10下无法使用问题，python版本需要3.5以上
版本：V1.0.4
日期：2019-07-11
作者: liansh
    2019-03-29 V1.0.0:
        首次发布
    2019-04-01 V1.0.1:
        1.修正debug.lkf文件生成顺序，完全按照CCS流程先加入默认Include目录，再写入链接参数。
    2019-04-03 V1.0.2:
        1.修正未识别指定lkf文件的问题
        2.修正前置、后置任务不区分Debug及Release模式的问题
    2019-05-27 V1.0.3
        1.增加对编译成功时，附带信息的支持(如#warn)
        2.取消无参数时进入调试模式
    2019-07-11 V1.0.4
        1.增加对打开优化的程序支持(特殊编译程序路径识别及切换)
"""
import os
import sys
import subprocess
import concurrent.futures   #线程池支持
import functools
from itertools import repeat
#from colorama import init, Fore, Back, Style
#from C2000_boot import boot

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

        self.initial_build_cmd_debug = []
        self.final_build_cmd_debug = []
        self.initial_build_cmd_release = []
        self.final_build_cmd_release = []

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
        if '["Debug" Settings]\n' in self.pjt_data:
            row = self.pjt_data.index('["Debug" Settings]\n')
            for i in self.pjt_data[row+1 : ]:
                if '\n' == i:
                    return
                if 'FinalBuildCmd' in i:        #后置任务
                    self.final_build_cmd_debug.append(i[ : -1])
                elif 'InitialBuildCmd' in i:    #前置任务
                    self.initial_build_cmd_debug.append(i[ : -1])
        if '["Release" Settings]\n' in self.pjt_data:
            row = self.pjt_data.index('["Debug" Settings]\n')
            for i in self.pjt_data[row+1 : ]:
                if '\n' == i:
                    return
                if 'FinalBuildCmd' in i:        #后置任务
                    self.final_build_cmd_release.append(i[ : -1])
                elif 'InitialBuildCmd' in i:    #前置任务
                    self.initial_build_cmd_release.append(i[ : -1])

#编译器
#work_dir : 工程所在目录
#c2000_dir: C2000软件包所在目录
#mode     : 编译模式(DEBUG, RELEASE)
#works    : 多线程编译线程数
class Compiler(object):
    def __init__(self, work_dir, c2000_dir, mode='DEBUG', works=4):
        self.mode = mode.upper()
        self.lkf_file = None
        self.build_log = []
        self.c2000_dir = c2000_dir
        self.work_dir = work_dir
        self.obj_dir = None
        try:
            self.max_works = int(works)
        except:
            self.max_works = 4

        self.proj = PJT(work_dir)

        #检查是否有临时目录，如果没有则创建，并清空临时目录
        #/q不需要手动确认 /f删除只读文件
        if not os.path.exists(self.proj.tmp_dir):
            os.mkdir(self.proj.tmp_dir)
        subprocess.getstatusoutput('del /q /f ' + self.proj.tmp_dir)

        #初始进入工程路径
        os.chdir(self.work_dir)

    def run(self):
        if not self.proj.status:
            return

        #前置任务
        self.initial(self.proj, self.mode)

        #lkf文件检查(创建)
        self.create_lkf(self.proj, self.mode)

        #编译
        if not self.build(self.proj, self.mode):
            return

        #lvk文件生成
        self.update_lkf(self.proj, self.mode, self.build_log)

        #连接
        self.linker()

        #后置任务
        self.final(self.proj, self.mode)

        #调试在线更新
        #self.boot = boot(self.work_dir, os.path.join(self.c2000_dir, "cgtools\\bin"), )

    def create_lkf(self, proj, mode):
        """创建lkf文件"""
        if 'DEBUG' == mode.upper():
            compiler_config = proj.compiler_config_debug
        else:
            compiler_config = proj.linker_config_release
        if '-@' in compiler_config:         #已经指定了lkf文件目录
            start =  compiler_config.index('-@"')
            end = compiler_config[start+3 : ].index('"')
            self.lkf_file = compiler_config[start + 3 : start + end + 3]
        else:                               #未指定lkf文件目录
            self.lkf_file =os.path.join(proj.tmp_dir, mode.lower() + '.lkf')
        #写入初始的默认include目录
        with open(self.lkf_file, 'w', encoding='utf-8') as file:
            inc_a = '-i"' + os.path.join(self.c2000_dir, 'xdais\include"')
            inc_b = '-i"' + os.path.join(self.c2000_dir, 'cgtools\include"')
            file.write(inc_a + ' ' + inc_b + '\n')

    def update_lkf(self, proj, mode, build_log):
        """编译完成后，更新lkf文件内容"""
        if 'DEBUG' == mode.upper():
            linker_config = proj.linker_config_debug
        else:
            linker_config = proj.linker_config_release

        #CCS3.3自带lib目录加入
        linker_config += ' -i"' + os.path.join(self.c2000_dir, 'xdais\lib') + '"'
        linker_config += ' -i"' + os.path.join(self.c2000_dir, 'cgtools\lib') + '"\n'

        with open(self.lkf_file, 'w', encoding='utf-8') as file:
            #首先清空lkf文件(编译时加入的默认include目录)
            file.write('')
            #写入Linker参数,加z意义不明,CCS中找不到相关设置
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
            self.obj_dir = proj.obj_dir_debug
        else:
            build_config = proj.compiler_config_release + ' '
            self.obj_dir = proj.obj_dir_release

        """
            根据是否有优化，切换不同的工作路径
            对于打开的了优化的工程，需要调用c2000目录下test目录下的文件，推测过程为(实际过程不明):
            cl2000生成.if文件，opt2000调用.if生成临时文件，ac2000再对临时文件进行处理，再生成.obj文件。
            但是可以直接进入test目录下完成编译，结果与CCS3.3一致。
        """
        if '-o' in build_config:        #需要优化的项目
            os.chdir(os.path.join(self.c2000_dir, 'cgtools\\bin\\test'))
        else:                           #无需优化的项目
            os.chdir(os.path.join(self.c2000_dir, 'cgtools\\bin'))
        bin_dir = '"' + os.path.join(self.c2000_dir, 'cgtools\\bin\\cl2000') + '" '
        
        #lkf_dir = ''
        #lkf必须要编译一条写入一条(待增加功能)
        lkf_dir = '-@"' + self.lkf_file + '" '
        cmd = bin_dir + build_config + lkf_dir

        #使用线程池来加速文件编译
        #在使用map时，线程池返回顺序与启动顺序相同，可以确保.lkf文件中OBJ文件顺序与实际待编译文件顺序一致
        #线程池中线程同时传入固定参数及list参数时，可以使用functools.partial或repeat来指定固定参数
        #但是注意，必须是可变参数在前，固定参数在后
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.max_works) as executor:
            #for status, output in zip(proj.c_file, executor.map(functools.partial(cl2000_shell, cc=cmd), proj.c_file)):
            for status, output in zip(proj.c_file, executor.map(cl2000_shell, proj.c_file, repeat(cmd))):
                if 0 == output[0]:
                    obj_file = os.path.join(self.obj_dir, status[status.rfind('\\')+1 : status.rfind('.')]) + '.obj'
                    self.build_log.append('"' + obj_file + '"\n')
                    if len(output[1]) > 0:              #2019-05-27 增加对编译成功时，附带信息的支持(#warn)
                        for i in output[1].split('\n'): #2019-05-28 output中可能包含较多行信息，使用'\n'区分
                            print(status + i[i.rfind(',') : ])
                            #print(Fore.RED + 'test' + Fore.RESET)  #VSCode 控制台无法显示颜色
                    else:
                        print(status + ' success --' + mode)
                else:
                    print(output[1])
                    return(False)
        return(True)

    def linker(self):
        #190711 由于编译的时候会进入c2000目录，所以在链接时重新回到工程目录
        os.chdir(self.work_dir)     

        """链接"""
        bin_dir = '"' + os.path.join(self.c2000_dir, 'cgtools\\bin\\cl2000') + '" '
        cmd = bin_dir + '-@"' + self.lkf_file + '"'
        status, output = subprocess.getstatusoutput(cmd)
        print('[Linking...] ' +  self.lkf_file)
        print(output)

    def initial(self, proj, mode):
        """前置任务"""
        pass
    def final(self, proj, mode):
        """后置任务"""
        if 'DEBUG' == mode:
            build_cmd = proj.final_build_cmd_debug
        else:
            build_cmd = proj.final_build_cmd_release
        for i in build_cmd:
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
        end_time = time.time()
        print("编译用时: {0:.2f} 秒".format(end_time - start_time))
    else:                       #无参数,默认调试
        #print("Parameter cannot be empty")
        work_path = 'E:\\Project\\20KW_PFC_2806\\20KW_PFC_TMS320F2806'
        c2000_path = 'E:\\Python\\Project\\C2000 Compiler\\C2000'
        cc = Compiler(work_path, c2000_path, works=1)
        cc.run()
    sys.exit()
