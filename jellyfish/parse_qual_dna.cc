/*  This file is part of Jellyfish.

    Jellyfish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jellyfish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jellyfish.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <jellyfish/parse_qual_dna.hpp>

namespace jellyfish {
  parse_qual_dna::parse_qual_dna(int nb_files, char *argv[], uint_t _mer_len,
                                 unsigned int nb_buffers, size_t _buffer_size,
                                 const char _qs, const char _min_q) :
    double_fifo_input(nb_buffers), mer_len(_mer_len), 
    buffer_size(_buffer_size), files(argv, argv + nb_files),
    current_file(files.begin()), have_seam(false), quality_start(_qs),
    min_q(_min_q)
  {
    buffer_data = new char[nb_buffers * buffer_size];
    seam        = new char[2*mer_len];

    unsigned long i = 0;
    for(bucket_iterator it = bucket_begin();
        it != bucket_end(); ++it, ++i) {
      it->end = it->start = buffer_data + i * buffer_size;
    }

    fparser =
      jellyfish::file_parser::new_file_parser_seq_qual(*current_file);
  }

  void parse_qual_dna::fill() {
    sequence_t *new_seq = 0;
  
    while(true) {
      if(!new_seq) {
        new_seq = wq.dequeue();
        if(!new_seq)
          break;
      }
      new_seq->end = new_seq->start + buffer_size;
      char *start = new_seq->start;
      if(have_seam) {
        have_seam = false;
        memcpy(start, seam, 2 * (mer_len - 1));
        start += 2 * (mer_len - 1);
      }
      bool input_eof = !fparser->parse(start, &new_seq->end);
      if(new_seq->end > new_seq->start + 2 * mer_len) {
        have_seam = true;
        memcpy(seam, new_seq->end - 2 * (mer_len - 1), 2 * (mer_len - 1));
        rq.enqueue(new_seq);
        new_seq = 0;
      }
      if(input_eof) {
        delete fparser;
        have_seam = false;
        if(++current_file == files.end()) {
          rq.close();
          break;
        }
        fparser = 
          jellyfish::file_parser::new_file_parser_seq_qual(*current_file);
      }
    }
  }

  const uint_t parse_qual_dna::codes[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1,  0, -1,  1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1,  0, -1,  1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
  };
}
