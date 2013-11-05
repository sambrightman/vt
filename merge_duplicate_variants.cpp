/* The MIT License

   Copyright (c) 2013 Adrian Tan <atks@umich.edu>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include "merge_duplicate_variants.h"

namespace
{

class Igor : Program
{
    public:

    ///////////
    //options//
    ///////////
    std::string input_vcf_file;
    std::string output_vcf_file;
    std::vector<GenomeInterval> intervals;
    bool merge_by_pos;

    ///////
    //i/o//
    ///////
    OrderedReader *odr;
    //OrderedReader *odr;
    vcfFile *ovcf;

    std::vector<bcf1_t*> pool;
        
    /////////
    //stats//
    /////////
    uint32_t no_total_variants;
    uint32_t no_unique_variants;
    
    /////////
    //tools//
    /////////
    VariantManip *var_manip;

    Igor(int argc, char **argv)
    {
        version = "0.57";

        //////////////////////////
        //options initialization//
        //////////////////////////
        try
        {
            std::string desc = "Merges duplicate variants by position with the option of considering alleles.  (This just discards the duplicate variant that appears later in the VCF file)";

            TCLAP::CmdLine cmd(desc, ' ', version);
            VTOutput my;
            cmd.setOutput(&my);
            TCLAP::ValueArg<std::string> arg_intervals("i", "i", "intervals []", false, "", "str", cmd);
            TCLAP::ValueArg<std::string> arg_interval_list("I", "I", "file containing list of intervals []", false, "", "file", cmd);
            TCLAP::ValueArg<std::string> arg_output_vcf_file("o", "o", "output VCF file [-]", false, "-", "str", cmd);
            TCLAP::SwitchArg arg_merge_by_position("p", "merge-by-position", "Merge by position [false]", cmd, false);
            TCLAP::UnlabeledValueArg<std::string> arg_input_vcf_file("<in.vcf>", "input VCF file", true, "","file", cmd);

            cmd.parse(argc, argv);

            input_vcf_file = arg_input_vcf_file.getValue();
            output_vcf_file = arg_output_vcf_file.getValue();
            merge_by_pos = arg_merge_by_position.getValue();
            parse_intervals(intervals, arg_interval_list.getValue(), arg_intervals.getValue());
        }
        catch (TCLAP::ArgException &e)
        {
            std::cerr << "error: " << e.error() << " for arg " << e.argId() << "\n";
            abort();
        }
    };
 
    void initialize()
    {
        //////////////////////
        //i/o initialization//
        //////////////////////
        odr = new OrderedReader(input_vcf_file, intervals);
        ovcf = NULL;
        std::cerr << output_vcf_file << "\n";
        ovcf = vcf_open(output_vcf_file.c_str(), "w");
        if (ovcf == NULL)
        {
            std::cerr <<" ovcf null\n";
        }
        //vcf_hdr_write(ovcf, sr->hdrs[0]);
        
        ////////////////////////
        //stats initialization//
        ////////////////////////
        no_total_variants = 0;
        no_unique_variants = 0;

        ////////////////////////
        //tools initialization//
        ////////////////////////
    }
    
    void merge_duplicate_variants()
    {
        std::map<std::string, uint32_t> variants;
        std::stringstream ss;
        uint32_t no_unique_variants = 0;
        uint32_t no_total_variants = 0;
        
        kstring_t var;
        var.s = 0;
        var.l = var.m = 0;
        
        std::map<std::string, std::vector<bcfptr>> m;
        std::vector<bcfptr> recs;
        
        bcf1_t * v;
        
        while (odr->read(v))
        {
            bcf_hdr_t *h = odr->get_hdr();
			bcf_get_variant(h, v, &var);
			
			const char* chrom = bcf_get_chrom(h, v);
            uint32_t pos1 = bcf_get_pos1(v);
            
			m.clear();
			
//	        if (m.find(var.s)!=m.end())
//            {
//        		vcf_write1(ovcf, h, v);
//        		m[var.s].push_back(recs[i]);
//        		++no_unique_variants;  
//            }
            
            ++no_total_variants;
                
        }

        vcf_close(ovcf);    
    };

    void print_options()
    {
        std::clog << "merge_duplicate_variants v" << version << "\n\n";

        std::clog << "options:     input VCF file        " << input_vcf_file << "\n";
        std::clog << "         [o] output VCF file       " << output_vcf_file << "\n";
        std::clog << "         [p] Merge by              " << merge_by_pos << "\n";
        std::clog << "         [i] intervals             " << intervals.size() <<  " intervals\n";
        std::clog << "\n";
    }

    void print_stats()
    {
        std::clog << "stats: Total Number of Observed Variants   " << no_total_variants << "\n";
        std::clog << "       Total Number of Unique Variants     " << no_unique_variants << "\n\n";
    };

    ~Igor() {};

    private:
};

}

void merge_duplicate_variants(int argc, char ** argv)
{
    Igor igor(argc, argv);
    igor.print_options();
    igor.initialize();
    igor.merge_duplicate_variants();
    igor.print_stats();
};