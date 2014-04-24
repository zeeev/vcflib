#include <iostream>
#include <fstream>
#include <getopt.h>
#include <map>
#include <list>
#include <vector>
#include <string>
#include "split.h"
#include <stdio.h> 
#include <stdlib.h>

using namespace std;

struct opts{
  string   format;
  long int step;
  long int size;
  int      seqid;
  int      pos  ; 
  int      value;
};

struct score{
  long int position;
  double score;
};

double windowAvg(list<score> & rangeData){

  double n = 0;
  double s = 0;

  for(list<score>::iterator it = rangeData.begin(); it != rangeData.end(); it++){
    s += it->score;
    n += 1;
  }
  return (s/n);
}

void processSeqid(ifstream & file, string seqid, streampos offset, opts & opt){
    
  string line ;
  
  long int windowSize = opt.size;
  long int start = 0;
  long int end   = windowSize;

  list<score> windowDat;

  file.clear();
    
  file.seekg(offset);
  
  vector<string> sline;

  while(getline(file, line)){
    
    sline = split(line, '\t');     
    score current ;
    if(seqid != sline[opt.seqid]){
      break;
    }
    current.position = atol( sline[opt.pos].c_str() );
    current.score    = atof( sline[opt.value].c_str() );
    
    if(current.position > end){
      double mean = windowAvg(windowDat);
      cout << seqid << "\t" << start << "\t" << end << "\t" << windowDat.size() << "\t" << mean << endl;
    }
    while(end < current.position){
      start += opt.step;
      end   += opt.step;
      while(windowDat.front().position < start && !windowDat.empty()){
	windowDat.pop_front();
      }
    }
    windowDat.push_back(current);  
  }
  double finalMean = windowAvg(windowDat);
  cout << seqid << "\t" << start << "\t" << end << "\t" << windowDat.size() << "\t" << finalMean << endl;
  cerr << "INFO: smoother finished : " << seqid << endl;
}

int main(int argc, char** argv) {
  
  map<string, int> acceptableFormats;
  acceptableFormats["pFst"]  = 1;
  acceptableFormats["bFst"]  = 1;
  acceptableFormats["wcFst"] = 1;
  

  opts opt;
  opt.size = 5000;
  opt.step = 1000;
  opt.format = "NA";

  string filename = "NA";

  static struct option longopts[] = 
    {
      {"version"   , 0, 0, 'v'},
      {"help"      , 0, 0, 'h'},
      {"file"      , 1, 0, 'f'},
      {"window"    , 1, 0, 'w'},
      {"step"      , 1, 0, 's'},
      {"format"    , 1, 0, 'o'},
      {0,0,0,0}
    };

  int index;
  int iarg=0;

  while(iarg != -1){
    iarg = getopt_long(argc, argv, "f:w:s:o:vh", longopts, &index);
    switch(iarg){
    case 'h':
      cerr << endl << endl;
      cerr << "INFO: help" << endl;
      cerr << "INFO: description:" << endl;
      cerr << "      smoother averages a set of values over a genomic range, window smoothing. " << endl;      
      cerr << "      The window size and step can be specified by the user. Smoother supports  " << endl;
      cerr << "      most of GPA++ formats and is specifed with the format flag                " << endl << endl;
      
      cerr << "Output : 4 columns :     "    << endl;
      cerr << "     1. seqid            "    << endl;
      cerr << "     2. position start   "    << endl;
      cerr << "     2. position end     "    << endl;
      cerr << "     3. averaged score   "    << endl  << endl;

      cerr << "INFO: usage: smoother --format pFst --file GPA.output.txt" << endl;
      cerr << endl;
      cerr << "INFO: required: f,file     -- a file created by GPA++"  << endl;
      cerr << "INFO: required: o,format   -- format of input file only: pFst or bFst, or wcFst" << endl;
      cerr << "INFO: optional: w,window   -- size of genomic window in base pairs (default 5000)"  << endl;
      cerr << "INFO: optional: s,step     -- window step size in base pairs (default 1000)" << endl;
      cerr << endl ;
      cerr << "INFO: version 1.0.0 ; date: April 2014 ; author: Zev Kronenberg; email : zev.kronenberg@utah.edu "  << endl;
      cerr << endl << endl;

      return 0;
    case 'v':
      cerr << endl << endl;
      cerr << "INFO: version 1.0.0 ; date: April 2014 ; author: Zev Kronenberg; email : zev.kronenberg@utah.edu "  << endl;
      return 0;
    case 'f':
      filename = optarg;
      cerr << "INFO: file : " << filename << endl;
      break;
    case 's':
      opt.step = atol(optarg);
      cerr << "INFO: step size : " << optarg << endl;
      break;
    case 'w':
      opt.size = atol(optarg);
      cerr << "INFO: step size : " << optarg << endl;
      break;
    case 'o':
      opt.format = optarg;
      cerr << "INFO: specified input format : " << optarg << endl;
      break;
    }
  }
  if(filename == "NA"){
    cerr << "FATAL: file was not specified!" << endl;
    cerr << "INFO:  please use smoother --help" << endl;
    return 1;
  }

  if(acceptableFormats.find(opt.format) == acceptableFormats.end()){
    cerr << "FATAL: input format flag not specified correctly : " << opt.format << endl;
    cerr << "INFO : acceptable options for --o : pFst | bFst | wcFst "  << endl;
    cerr << "INFO:  please use smoother --help" << endl;
    return 1;
  }
  
  if(opt.format == "pFst"){
    opt.seqid = 0;
    opt.pos   = 1;
    opt.value = 2;
  }
  else if (opt.format == "bFst"){
    opt.seqid = 0;
    opt.pos   = 1;
    opt.value = 8;
  }
  else if (opt.format == "wcFst"){
    opt.seqid = 0;
    opt.pos   = 1;
    opt.value = 2;
  }
  else{
    cerr << "FATAL: input format flag not specified correctly : " << opt.format << endl;
    cerr << "INFO:  please use smoother --help" << endl;
    return 1;
  }
  
  ifstream ifs(filename.c_str());
 
  string currentSeqid = "NA";

  string line;

  map<string, streampos > seqidIndex;
  
  if(ifs){
    while(getline(ifs, line)){
      vector<string> sline = split(line, '\t');
      if(sline[opt.seqid] != currentSeqid){
	
	int bline = ifs.tellg() ;
	//	bline -=  - ( line.size() +1 );
	
	map<string, streampos>::iterator it;

	if(seqidIndex.find(sline[opt.seqid]) != seqidIndex.end() ){
	  cerr << "FATAL: file is unsorted!" << endl;
	  return 1;
	}
	seqidIndex[sline[opt.seqid]] = bline;
	currentSeqid = sline[opt.seqid];
      }
    }
  }
  else{
    cerr << "FATAL: no lines -- or -- couldn't open file" << endl;
  }

  for( map< string, streampos>::iterator it = seqidIndex.begin(); it != seqidIndex.end(); it++){
    cerr << "INFO: processing seqid : "<< (it->first) << endl;
    processSeqid(ifs, (it->first),(it->second), opt);
  }
  
  ifs.close();
  cerr << "INFO: smoother has successfully finished" << endl;

  return 0;

}