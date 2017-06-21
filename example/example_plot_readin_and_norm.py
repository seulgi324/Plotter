#! /usr/bin/env python
from plotlib import HistStorage,getColorList,getDictValue,HistStorageContainer,getRGBTColor
import os,math
from DukePlotALot import *
from collections import OrderedDict

def getSampleInfo(configfile):
    lumi=1.
    sampleInfo={}
    basedir=""

    with open(configfile) as configFile:
        for line in configFile:
            if line[0]=="#" or line[0:1]=="//":
                continue
            elif "luminosity" in line:
                lumi=float(line.split()[1])
                continue
            elif ".root" in line and "output" not in line:
                tmp=line.strip().split()
                if(len(tmp)<2):
                    continue
                sample=tmp[0]
                if "/" in sample:
                    basedir=os.path.dirname(sample)
                    sample=os.path.basename(sample)
                sampleCombine=tmp[1]
                if "Data" in sampleCombine:
                    sampleInfo[sample]={
                                        "output":sampleCombine,
                                        "lumi":1.,
                                        "xs":1.,
                                        "eff":1.,
                                        "type":"data",
                                        }
                elif(len(tmp)>4):
                    sampleInfo[sample]={
                                        "output":sampleCombine,
                                        "lumi":lumi,
                                        "xs":float(tmp[2]),
                                        "eff":float(tmp[3]),
                                        "type":tmp[4],
                                        }
    return sampleInfo,basedir


def main():
    #get the list of bg and data files from the unusal config
    sampleinfos,basedir=getSampleInfo("201705_mergedext.config")
    lumi=sampleinfos.values()[-1]["lumi"]
    useRoot=False

    #we will use the same colors as the .c plotter for now
    colors=[100, 90, 80, 70, 60, 95, 85, 75, 65, 55]
    #reverse order and change to RGB
    colors=[getRGBTColor(i) for i in colors[::-1]]

    #make storage containers for the bgs and data
    bghists=HistStorage(xs=sampleinfos,xstype="BSM3G",lumi=lumi,path=basedir,useRoot=useRoot)
    dat_hist=HistStorage(path=basedir,isData=True,useRoot=useRoot)

    #make some lists that we will put into the containers
    colorList={}
    bglist=OrderedDict()

    #the idea is to have a readable dict:

    bglist['DY+Jets']=[
        'DYJetsToLL_M-50_HT-70To100',
        'DYJetsToLL_M-50_HT-600To800',
        'DYJetsToLL_M-50_HT-400To600',
        'DYJetsToLL_M-50_HT-800To1200',
        'DYJetsToLL_M-50_HT-1200To2500',
        'DYJetsToLL_M-50_HT-2500ToInf',
        'DYJetsToLL_M-50_HT-200To400',
        'DYJetsToLL_M-50_HT-0To70',
        'DYJetsToLL_M-50_HT-100To200',
        ]
    bglist['VV']=[
        'ZZ_TuneCUETP8M1_13TeV',
        'WZ_TuneCUETP8M1_13TeV',
        'WW_TuneCUETP8M1_13TeV',
        ]
    bglist['SingleTop']=[
        'ST_tW_antitop_5f_inclusiveDecays_13TeV',
        'ST_t-channel_top_4f_inclusiveDecays_13TeV',
        'ST_tW_top_5f_inclusiveDecays_13TeV',
        'ST_t-channel_antitop_4f_inclusiveDecays_13TeV',
        ]
    bglist['$\mathsf{t\bar{t}}$']=[
        'TT_TuneCUETP8M2T4_13TeV',
        ]
    bglist['W+Jets']=[
        'WJetsToLNu_HT-0To70',
    ]

    ##you can reverse the order
    bglist=OrderedDict(reversed(list(bglist.items())))

    #now actually add the backgrounds
    bghists.addFileList(bglist)
    bghists.colorList=colorList

    hists=[
        "NDiMuonCombinations/MHT",
        "NDiMuonCombinations/Meff",
    ]

    histContainer=HistStorageContainer(bg=bghists,data=dat_hist)
    #histContainer=HistStorageContainer(bg=bghists,sg=sghist,data=dat_hist)



    bghists.initStyle(style="bg")
    for hist in hists:
        #this is the magic line
        histContainer.getHist(hist)

        #now you can do what you want with the hists
        allbg=sum(histContainer.getBGList())
        allbg.Draw()
        for bg in histContainer.getBGList():
            bg.Draw("same")
        data=histContainer.getData()
        data.Draw("same p")

    ##an other example is the tree stuff

    histContainer.getHistFromTree2d(500,0,5000,r"k-factor",1000,0,2,r"M_{T} [GeV]" ,"ThisWeight*(lepton_type==11)*(mt>50)*(pt/met<1.5)*(pt/met>0.4)*(delta_phi>2.4)","mt:kfak","Trees/slimtree")
    #this works like:
    #getHistFromTree(bins,xmin,xmax,xtitle,cut,value,tree,weight=None)
    #and
    #getHistFromTree2d(xbins,xmin,xmax,xtitle,ybins,ymin,ymax,ytitle,cut,value,tree,weight=None)
    #where value has to be a string "y:x" (root logic)







if __name__ == '__main__':
    main()
