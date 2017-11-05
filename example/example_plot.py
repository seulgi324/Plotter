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
    for sample in sampleinfos:
        info=sampleinfos[sample]
        sample=sample.replace(".root","")
        if info["type"]=="bg":
            shortName=info["output"].replace(".root","")
            if "#" in shortName:
                shortName="$\mathsf{"+shortName.replace("#","\\")+"}$"
            if shortName not in bglist:
                bglist[shortName]=[sample]
                colorList[shortName]=colors.pop()
            else:
                bglist[shortName].append(sample)
        #for data directly add the file
        if info["type"]=="data":
            dat_hist.addFile(sample)
    for bg in bglist:
        print("bglist['%s']=["%(bg))
        for ibg in bglist[bg]:
            print("'ibg',")
        print("]")
        
    
    #now actually add the backgrounds
    bghists.addFileList(bglist)
    bghists.colorList=colorList

    hists=[
        "NDiMuonCombinations/MHT",
        "NDiMuonCombinations/Meff",
    ]

    histContainer=HistStorageContainer(bg=bghists,data=dat_hist)
    #histContainer=HistStorageContainer(bg=bghists,sg=sghist,data=dat_hist)



    binning={
            "_pt":25,
            "_mt":25,
            "_met":25,
            "MC_W_m_Gen":10,
            "_jet_ht":10,
    }

    xranges={
            "_pt":[0,1000],
            "_jet_ht":[0,1600],
            "_mt":[150,1600],
            "boson_qt":[0,600],
            "_met":[0,1300],
            "_ET_MET":[0,4],
            "_DeltaPhi":[0.2,math.pi],
            "MC_W_m_Gen":[0,1300],
            "relIso":[0,0.05],
    }
    yranges={
            "_ET_MET":[1.01e-3,1e8],
            "_DeltaPhi":[1.01e-3,1e8],
    }

    bghists.initStyle(style="bg")
    #sghist.initStyle(style="sg")
    for hist in hists:

        ##use different out dirs
        dir_name="test"
        if not os.path.exists(dir_name):
            os.mkdir(dir_name)

        if "prof" in hist:
            hist_style = sc.style_container(kind="Lines",style = 'CMS', useRoot=useRoot,cmsPositon="upper left",lumi=lumi,cms=13)
        else:
            hist_style = sc.style_container(style = 'CMS', useRoot=useRoot,cmsPositon="upper left",lumi=lumi,cms=13)

        #change the style if you want
        #hist_style._error_bands_fcol=["grey","yellow"]
        #hist_style._error_bands_ecol=["lightgrey","yellow"]
        #hist_style._error_bands_alph=0.4
        #hist_style.Set_error_line_width(0.0000000001)
        hist_style.Set_poisson_error_off()
        #hist_style.Set_minor_ticks()
        #hist_style.Set_n_legend_columns(2)
        #hist_style.Set_xerr()
        #hist_style.Set_do_overflowbin()

        #get the actual histograms
        histContainer.getHist(hist)


        #programmable binning
        binf=getDictValue(hist,binning)
        if binf is not None:
            if isinstance(binf,list):
                histContainer.rebin(vector=binf)
            else:
                histContainer.rebin(width=binf)

        name=hist.replace("/","")
        #test = plotter(hist=histContainer.getBGList(),data_hist=histContainer.getData(),data=True,style=hist_style)
        test = plotter(hist=histContainer.getBGList(),style=hist_style)
        test.Add_data(histContainer.getData())


        test.Add_plot('Ratio',pos=1, height=15)
        test.Add_plot('DiffRatio_width_increase',pos=2, height=15)
        mplt=test.create_plot()
        test.SavePlot('%s/%s.png'%(dir_name,name))
        test.SavePlot('%s/%s.pdf'%(dir_name,name))






if __name__ == '__main__':
    main()
