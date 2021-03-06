<?php

/**
 * @file
 */
class BiblioMigrateEntries extends Migration {

  public $dependencies = array('BiblioMigrateTypes');

  public function __construct() {
    $this->description = t('Migrate biblio 1.x entries.');

    $this->query = db_select('_biblio_1x', 'b');
    $this->query->innerJoin('node', 'n', 'n.nid = b.nid AND n.vid = b.vid');

    $this->query
      ->fields('b')
      ->fields('n', array('nid', 'title', 'created', 'uid'))
      ->condition('n.status', NODE_PUBLISHED)
      ->condition('b.biblio_type', 0, '>');

    parent::__construct();

    $source_key = array(
      'nid' => array(
        'type' => 'int',
        'not null' => TRUE,
        'alias' => 'n',
      ),
    );

    $this->map = new MigrateSQLMap($this->machineName, $source_key, MigrateDestinationEntityAPI::getKeySchema('biblio'));
    $this->source = new MigrateSourceSQL($this->query);

    // @todo: All bundles have all the field, so we hardcode the bundle. Is this
    // ok?
    $this->destination = new MigrateDestinationEntityAPI('biblio', 'book');

    $fields = array(
      'title',
      'uid',
      'created',
      'changed',
    );

    foreach (array_keys(biblio_fields_info()) as $field_name) {

      if (in_array($field_name, array('biblio_type', 'biblio_abstract', 'biblio_issued', 'biblio_status'))) {
        continue;
      }
      $fields[] = $field_name;
    }

    $this->addSimpleMappings($fields);

    $this
      ->addFieldMapping('type', 'biblio_type')
      ->sourceMigration('BiblioMigrateTypes');

    $this
      ->addFieldMapping('biblio_abstract', 'biblio_abst_e')
      ->defaultValue(NULL);

    $this
      ->addFieldMapping('biblio_status', 'biblio_year')
      ->callbacks(array($this, 'prepareBiblioStatus'));
  }

  public function prepareBiblioStatus($value) {
    switch ($value) {
      case '9998':
        return 'in_press';

      case '9999':
        return 'submitted';

      default:
        return 'published';
    }
  }
}
