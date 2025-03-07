#! /usr/bin/env python3

import os
import sys
import yaml

this_dir = os.path.dirname(os.path.realpath(__file__))

def main():
    grammar_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(this_dir, 'kimchi-grammar')
    point_dir = os.path.join(grammar_dir, 'point')
    point_files = [f for f in os.listdir(point_dir) if f.endswith('.yaml')]

    num_total = 0
    num_handled = 0

    types = set()

    out_path = os.path.join(this_dir, 'detect-rules')

    with open(out_path, 'w') as fout:

        for point_file in point_files:
            num_total += 1
            point_path = os.path.join(point_dir, point_file)

            slug = point_file.replace('.yaml', '')

            with open(point_path, 'r') as fin:
                point = yaml.load(fin, Loader=yaml.FullLoader)

            type_ = point['metadata']['type']
            detect = point['metadata'].get('detect')
            post_ruleset = point['metadata'].get('detect_post', '')
            types.add(type_)

            if detect:
                fout.write(f'{slug}:{type_[0]}:{post_ruleset}:{detect}\n')
                num_handled += 1
    
    print(f'Handled {num_handled}/{num_total} points')


if __name__ == '__main__':
    main()
