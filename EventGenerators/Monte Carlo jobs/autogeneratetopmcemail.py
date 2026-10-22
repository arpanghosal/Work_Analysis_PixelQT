import sys, os, subprocess, shutil, glob
import requests

if os.path.isdir("mcjoboptions"):
    shutil.rmtree("mcjoboptions")
p = subprocess.Popen(["git","clone","--depth=1","https://:@gitlab.cern.ch:8443/atlas-physics/pmg/mcjoboptions.git"])
p.wait()

def get_joboption_from_dsid(dsid):

    joboption = "Not found!"
    for filename in glob.glob("mcjoboptions/"+dsid[:3]+"xxx/"+dsid+"/*"):
        filename = filename.split("/")[-1]
        if filename.startswith("mc.") and filename.endswith(".py"):
            joboption = filename

    return joboption

def read_spreadsheet_and_write_text(spreadsheet, campaign, general_comments, jira_link, do_only_mc16a=False):

    text = ""

    response = requests.get(spreadsheet+'/export?format=csv')
    assert response.status_code == 200, 'Wrong status code'

    content = response.text.split("\n")
    headers = content[0]

    samples = []

    for sample_line in content[1:]:
        sample_info = sample_line.split(',')
        dsid        = sample_info[0]
        inputfile   = sample_info[1]
        com_energy  = sample_info[2]
        nevents     = sample_info[3]
        gentype     = sample_info[4]
        if gentype == "EVGEN" or gentype == "Evgen":
            gentype = "EVGEN-only"
        elif gentype == "FullSim":
            gentype = "EVGEN+FullSim"
        elif gentype == "AF2":
            gentype = "EVGEN+FastSim"
        priority    = sample_info[5]
        outputformat= sample_info[6]
        release     = sample_info[7]
        comments    = sample_info[8]
        if "from existing evgen" in comments or "from existing EVNT" in comments or "from existing Evgen" in comments:
            gentype = gentype.split("+")[1]
        evgen_tag   = sample_info[9]
        simul_tag   = sample_info[11]
        digi_tag    = sample_info[13]
        reco_tag    = sample_info[14]
        reco_mertag = sample_info[15]
        joboption   = get_joboption_from_dsid(dsid)
        samples.append([joboption,dsid,inputfile,com_energy,nevents,gentype,priority,outputformat,release,comments,evgen_tag,simul_tag,digi_tag,reco_tag,reco_mertag])

    priorities = list(set([sample[6] for sample in samples]))

    text += "*"*20+campaign+"*"*20+"\n"
    text += general_comments
    text += " This is an "+campaign+" request, for which we are requesting "
    if len(priorities)==1:
        text += "priority "+priorities[0]+". "
    else:
        text += "priorities "+"/".join(priorities)+". "
    if campaign == "mc16a":
        if do_only_mc16a:
            text +="\n"
        else:
            text += "Mc16d and mc16e will be requested soon.\n"
    elif campaign == "mc16d":
        text += "Mc16e will be requested soon.\n\n"
        text += "mc16a was requested here: \n"
    elif campaign == "mc16e":
        text += "\n\n"
        text += "mc16a was requested here: \n"
        text += "mc16d was requested here: \n"
    text += "\n"
    text += "Details:\n\n"
    total_events = 0
    gen_types = []
    for sample in samples:
        text += sample[1]+", "+sample[0].split(".")[1]+", "
        nevents  = int(float(sample[4]))
        gen_type = sample[5]
        save_LHE = ( sample[7] == "TXT" )
        total_events += nevents
        gen_types.append(gen_type)
        if nevents >= 1.e6:
            nevents = "{:.2f}M".format(nevents/1.e6)
        else:
            nevents = "{:.0f}k".format(nevents/1.e3)
        text += nevents+", "+gen_type
        if save_LHE:
            text += ", save output LHE"
        if len(priorities)>1:
            text += ", priority "+sample[6]
        text += "\n"

    text += "\n"
    input_files = [sample[2] for sample in samples if sample[2]]
    if input_files:
        text += "Input files:\n\n"
        for sample in samples:
            if sample[2]:
                text += sample[1]+": "+sample[2]+"\n"
        text += "\n"

    text += "Handshake: The top conveners signed-off on this request\n\n"
    text += "DSID(s): "+",".join(sample[1] for sample in samples)+"\n"
    text += "Events: "
    if total_events >= 1.e6:
        total_events = "{:.2f}M".format(total_events/1.e6)
    else:
        total_events = "{:.0f}k".format(total_events/1.e3)
    text += total_events+" events\n"
    text += "Priority: "
    if len(priorities)==1:
        text += priorities[0]+"\n"
    else:
        text += "/".join(priorities)+"\n"
    text += "Details: "+campaign+", AthGeneration-"+samples[0][8]+", "+"/".join(list(set(gen_types)))
    if input_files:
        text += ", use input TXT\n"
    else:
        text += "\n"
    text += "JIRA: "+jira_link+"\n"
    text += "Spreadsheet: "+spreadsheet+"\n\n"

    return text

if __name__=='__main__':
    spreadsheets = []
    with open("spreadsheets.txt","r") as f:
        for line in f: spreadsheets.append(line.rstrip())
    mc16a_only = False
    if len(spreadsheets)==1: mc16a_only = True
    general_comments = ""
    with open("comments_and_jira.txt","r") as f:
        lines            = f.readlines()
        general_comments = lines[0].rstrip()
        jira_ticket      = lines[1].rstrip()
    email_text = ""
    if mc16a_only:
        email_text += read_spreadsheet_and_write_text(spreadsheets[0], "mc16a", general_comments, jira_ticket, mc16a_only)
    else:
        for spreadsheet,campaign in zip(spreadsheets,["mc16a","mc16d","mc16e"]):
            email_text += read_spreadsheet_and_write_text(spreadsheet, campaign, general_comments, jira_ticket)
    with open("email_draft_"+jira_ticket.split("/")[-1]+".txt","w") as f:
        f.write(email_text)
    print("Done!")
