#!/usr/bin/env python
# -*- coding: utf-8

""" 190904
试验结果：bin生成时会自动计算并写入CRC校验。asm文件中的校验和仅用于仿真器烧录使用。
                因无方法简单更新.out文件中的校验和，此项目终止。
      end """ 

import sys, os
import subprocess
import struct

class hex_file(object):
    def __init__(self, file_dir):
        self.dir = file_dir

    def tobin(self):
        status, crc = hex2bin(self.dir)
        if 0 != status:
            return(1, 0, 0)
        else:
            return(0, self.dir + '.bin', crc)

class out_file(object):
    def __init__(self, file_dir, c2k_dir):
        self.file = file_dir
        self.c2k_dir = c2k_dir

    def tohex(self):
        if 0 != out2hex(self.file, self.c2k_dir):
            return(1, 0)
        else:
            return(0, self.file + '.hex')

class pjt(object):
    def __init__(self, proj_dir, c2k_dir, out_dir, lkf_file):
        self.proj_dir = proj_dir
        self.c2k_dir = c2k_dir
        self.lkf_file = lkf_file
        self.obj_file = os.path.join(self.proj_dir, 'OBJ\\DCDC100A_EntryVect.obj')
        self.out_file = os.path.join(self.proj_dir, out_dir)
        self.hex_file = None
        self.bin_file = None

    def cover_hex(self, mode = 'b'):
        if 0 != out2hex(self.out_file, self.c2k_dir, mode):
            return(1, 0)
        else:
            self.hex_file = self.out_file + '.hex'
            return(0, self.hex_file)

    def clean(self):
        if None != self.hex_file:
            subcmd('del ' + self.hex_file)
            subcmd('del ' + self.out_file)
            
    def cover_bin(self):
        s, c = hex2bin(self.hex_file)
        if 0 != s:
            return(1, 0, 0)
        else:
            self.bin_file = self.hex_file + '.bin'
            return(0, self.bin_file + '.bin', c)

    def obj_edit(self, crc):
        print(self.obj_file)
        crc_h = struct.pack("B", crc >> 8)
        crc_l = struct.pack("B", crc & 0xFF)
        with open(self.obj_file, 'r+b') as obj:
            obj.seek(0x31F)
            obj.write(crc_l)
            obj.seek(0x320)
            obj.write(crc_h)

    def relink(self):
        """链接"""
        print('[Re Linking...] ' +  self.lkf_file)
        s, o = link(self.c2k_dir, self.lkf_file)
        return(s)


class boot(object):
    def __init__(self, work_dir, c2k_dir, out_dir, lkf_file):
        self.status = 0

        subcmd('subst w: "' + work_dir + '"')               #创建一个镜像目录，代替工作目录，防止目录中空格造成的异常
        subcmd('subst u: "' + c2k_dir + '"' )
        
        self.pjt = pjt('w:\\', c2k_dir, os.path.join('w:\\', out_dir), os.path.join('w:\\', lkf_file))

    def run(self):
        os.chdir('u:\\')                                                #进入hex2000目录
        s, f = self.pjt.cover_hex()
        if 0 != s:
            print('fail-1')
            return

        os.chdir('w:\\')                                                #进入工程目录
        s, f, c = self.pjt.cover_bin()
        if 0 != s:
            print('fail-2')
            return
        print('CRC：{0:4X}'.format(c))
        self.pjt.obj_edit(c)

        self.pjt.clean()
        if 0 != self.pjt.relink():                                      #重新链接
            print('fail-3')
            return

        os.chdir('u:\\')                                                #进入hex2000目录
        self.pjt.cover_hex(mode = 'a')

    def exit(self):
        subcmd('subst w: /d')
        subcmd('subst u: /d')

def subcmd(cc):
    status, output = subprocess.getstatusoutput(cc)
    return(status, output)

def hex2bin(hex_file):
    cmd = 'ProcessBootImage280xx 2806  F28035_RAMBoot.hex'
    cmd += ' "' + hex_file + '"'
    cmd += ' "' +  hex_file + ".bin"
    status, output = subcmd(cmd)
    if 0 == status:
        crc = int(output[-5: -1], 16)
    else:
        crc = 0
    return(status, crc)

def out2hex(out_file, c2k_dir, mode):
    cmd = 'hex2000 -i -memwidth=16 -romwidth=16'
    if 'b' == mode:
        cmd += ' -byte  -exclude=appcrc -exclude=.bootvec -exclude=.cal -exclude=bootramfuncs -exclude=Flash28_API -exclude=.init -exclude=FlashBoot -exclude=csm_rsvd -exclude=codestart -exclude=csmpasswds'
    cmd += ' -o"' + out_file + '.hex"'
    cmd += ' "'+ out_file + '"'
    print(cmd)
    status, output = subcmd(cmd)
    return(status)

def link(c2k_dir, lkf_file):
        bin_dir = '"' + os.path.join(c2k_dir, 'cl2000') + '" '
        cmd = bin_dir + '-@"' + lkf_file + '"'
        s, o = subprocess.getstatusoutput(cmd)
        return(s, o)

if __name__ == '__main__':
    c = boot("E:\\Project\\20KW_DCDC\\20kw_dcdc_1016-0636",  "E:\\Python\\Project\\C2000 Compiler\\C2000\\cgtools\\bin", "debug\\1016_0636_DCDC.out", "tmp\\debug.lkf")
    c.run()
    c.exit()
