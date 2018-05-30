import pandas as pd
from matplotlib import pyplot


def plot(key_x:str,key_y:str,x_label:str,y_label:str,title:str,file_label_dict,dest:str,log=None):
    if log:
        pyplot.xscale("log")
    for label,file_name in label_file_dict.items():
        data=pd.read_csv(file_name)
        x=data[key_x].values
        y=data[key_y].values
        pyplot.scatter(x,y,label=label)
    pyplot.title(title)
    pyplot.xlabel(x_label)
    pyplot.ylabel(y_label)
    pyplot.legend()
    pyplot.savefig(dest)
    pyplot.figure()

"""
dest='4core_varying_data.eps'
base_name='core4_nt40'
dir_name='log/'
label_file_dict={'single':dir_name+'seq_'+base_name,'x':dir_name+'x_'+base_name,'y':dir_name+'y_'+base_name}
plot('NX','Speed(GFlops)','size','Gflops','data_size and Gflops on 4threads',label_file_dict,dest)


dest='varying_core.eps'
base_name='y_index13_core'
label_file_dict={'y':dir_name+'y_index13_core'}
plot('Core','Speed(GFlops)','threads','Gflops','num of threads and Gflops on 8192',label_file_dict,dest)


dir_name='sort/'
dest='sort_core.eps'
base_name='core_2-28'
label_file_dict={'':dir_name+base_name}
plot('core','time(us)','thread','time(us)','num of threads and time on 10**7',label_file_dict,dest)

dest='sort_task_limit.eps'
base_name='task_limit'
label_file_dict={'':dir_name+base_name}
plot('task_limit','time(us)','task_limit(log)','time(us)','task_limit(log) and time on 10**7',label_file_dict,dest,log=True)
"""

dest='varying_data.eps'
base_name='proc28_vsize'
dir_name='log/'
label_file_dict={'28core':dir_name+base_name}
plot('NX','Speed(GFlops)','size','Gflops','data_size and Gflops on 28procs',label_file_dict,dest,log=True)


dest='varying_core.eps'
base_name='size13_vproc'
label_file_dict={'2^13size':dir_name+base_name}
plot('Size','Speed(GFlops)','procs','Gflops','num of process and Gflops on 8192',label_file_dict,dest)

